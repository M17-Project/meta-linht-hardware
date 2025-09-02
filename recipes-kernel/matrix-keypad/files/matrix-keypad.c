/*
 * Based on the OpenRTX implementation of the C62 keyboard
 * https://github.com/OpenRTX/OpenRTX/
 * Released under GPL v3
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

#define DRIVER_NAME "matrix-keypad"
#define POLL_INTERVAL_MS 20 // Poll every 20ms for debouncing

// Based on the original Zephyr code (5 rows, 4 columns)
#define MATRIX_ROWS 5
#define MATRIX_COLS 4

/*
 * This keymap translates the raw bit position from the scan (row * num_cols + col)
 * to a Linux keycode defined in <linux/input-event-codes.h>.
 * The original KEY_MONI is mapped to KEY_F1 as an example.
 */
static const uint32_t matrix_keycodes[MATRIX_ROWS][MATRIX_COLS] = {
    { KEY_ENTER,  KEY_UP,        KEY_DOWN,     KEY_ESC    },
    { KEY_F1,     KEY_F2,        KEY_1,        KEY_2      },
    { KEY_3,      KEY_4,         KEY_5,        KEY_6      }, // KEY_STAR
    { KEY_7,      KEY_8,         KEY_9,        KEY_KPASTERISK },
    { KEY_0,     KEY_BACKSLASH,        KEY_RESERVED,        KEY_RESERVED      }  // KEY_MONI maps to F1
};

/*
static const uint32_t matrix_keycodes[MATRIX_ROWS][MATRIX_COLS] = {
    { KEY_ENTER,  KEY_UP,        KEY_DOWN,     KEY_ESC    },
    { KEY_3,      KEY_4,         KEY_5,        KEY_6      },
    { KEY_7,      KEY_8,         KEY_9,        KEY_KPASTERISK }, // KEY_STAR
    { KEY_0,      KEY_BACKSLASH, KEY_RESERVED, KEY_RESERVED },
    { KEY_F1,     KEY_F2,        KEY_1,        KEY_2      }  // KEY_MONI maps to F1
};

*/


struct matrix_keypad_data {
    struct input_dev *input_dev;
    struct device *dev;
    struct timer_list poll_timer;
    struct gpio_descs *row_gpios;
    struct gpio_descs *col_gpios;
    uint32_t last_key_state;
};

static void matrix_keypad_scan(struct timer_list *t)
{
    struct matrix_keypad_data *data = from_timer(data, t, poll_timer);
    struct input_dev *input_dev = data->input_dev;
    uint32_t current_key_state = 0;
    int r, c;

    // Scan the matrix
    for (r = 0; r < MATRIX_ROWS; r++) {
        // Drive the current row low
        gpiod_set_value(data->row_gpios->desc[r], 0);

        // A small delay for signal to propagate might be needed on some hardware
        // udelay(1);

        for (c = 0; c < MATRIX_COLS; c++) {
            // If a column reads low, the key is pressed
            if (gpiod_get_value(data->col_gpios->desc[c]) == 0) {
                current_key_state |= (1 << (r * MATRIX_COLS + c));
            }
        }
        // Set row back to high-impedance (or high)
        gpiod_set_value(data->row_gpios->desc[r], 1);
    }
    
    // Compare with the last state and report changes
    if (current_key_state != data->last_key_state) {
        uint32_t changed_bits = current_key_state ^ data->last_key_state;
        
        // Report matrix key changes
        for (r = 0; r < MATRIX_ROWS; r++) {
            for (c = 0; c < MATRIX_COLS; c++) {
                int bit_pos = r * MATRIX_COLS + c;
                if ((changed_bits >> bit_pos) & 1) {
                    bool pressed = (current_key_state >> bit_pos) & 1;
                    input_report_key(input_dev, matrix_keycodes[r][c], pressed);
                }
            }
        }

        input_sync(input_dev);
    }

    data->last_key_state = current_key_state;
    mod_timer(&data->poll_timer, jiffies + msecs_to_jiffies(POLL_INTERVAL_MS));
}

static int matrix_keypad_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct matrix_keypad_data *data;
    struct input_dev *input_dev;
    int r, c;
    int error;

    data = devm_kzalloc(dev, sizeof(struct matrix_keypad_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;
    
    data->dev = dev;

    // Parse GPIOs from Device Tree
    data->row_gpios = devm_gpiod_get_array(dev, "row", GPIOD_OUT_HIGH);
    if (IS_ERR(data->row_gpios)) {
        dev_err(dev, "Failed to get row gpios\n");
        return PTR_ERR(data->row_gpios);
    }

    data->col_gpios = devm_gpiod_get_array(dev, "col", GPIOD_IN);
    if (IS_ERR(data->col_gpios)) {
        dev_err(dev, "Failed to get col gpios\n");
        return PTR_ERR(data->col_gpios);
    }

    // Check if we got the correct number of GPIOs
    if (data->row_gpios->ndescs != MATRIX_ROWS || data->col_gpios->ndescs != MATRIX_COLS) {
        dev_err(dev, "Incorrect number of row/col gpios specified in DT\n");
        return -EINVAL;
    }
    
    // Do NOT configure pull-ups here; handle this in device tree with bias-pull-up properties.

    // Allocate and configure the input device
    input_dev = devm_input_allocate_device(dev);
    if (!input_dev)
        return -ENOMEM;

    data->input_dev = input_dev;
    input_dev->name = DRIVER_NAME;
    input_dev->phys = "matrix-keypad/input0";

    input_dev->id.bustype = BUS_HOST;
    input_dev->id.vendor = 0x0001;
    input_dev->id.product = 0x0001;
    input_dev->id.version = 0x0100;

    // Declare which events can be generated
    input_set_capability(input_dev, EV_KEY, KEY_RESERVED); // Base capability
    for (r = 0; r < MATRIX_ROWS; r++) {
        for (c = 0; c < MATRIX_COLS; c++) {
            if (matrix_keycodes[r][c] != KEY_RESERVED)
                input_set_capability(input_dev, EV_KEY, matrix_keycodes[r][c]);
        }
    }

    // Set up and start the polling timer
    timer_setup(&data->poll_timer, matrix_keypad_scan, 0);
    data->poll_timer.expires = jiffies + msecs_to_jiffies(POLL_INTERVAL_MS);
    add_timer(&data->poll_timer);

    // Register the input device
    error = input_register_device(input_dev);
    if (error) {
        dev_err(dev, "Failed to register input device\n");
        del_timer_sync(&data->poll_timer);
        return error;
    }

    platform_set_drvdata(pdev, data);
    dev_info(dev, "Matrix keypad driver loaded\n");

    return 0;
}

static int matrix_keypad_remove(struct platform_device *pdev)
{
    struct matrix_keypad_data *data = platform_get_drvdata(pdev);

    // Stop the timer. The input device and GPIOs are managed by devm_* and will be freed automatically.
    del_timer_sync(&data->poll_timer);
    dev_info(&pdev->dev, "Matrix keypad driver unloaded\n");
    return 0;
}

// Match the driver with the device tree "compatible" property
static const struct of_device_id matrix_keypad_of_match[] = {
    { .compatible = "linht,matrix-keypad" },
    {}
};
MODULE_DEVICE_TABLE(of, matrix_keypad_of_match);

static struct platform_driver matrix_keypad_driver = {
    .probe = matrix_keypad_probe,
    .remove = matrix_keypad_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = matrix_keypad_of_match,
    },
};

module_platform_driver(matrix_keypad_driver);

MODULE_AUTHOR("Andreas Schmidberger, OE3ANC");
MODULE_DESCRIPTION("Matrix Keyboard Driver");
MODULE_LICENSE("GPL");