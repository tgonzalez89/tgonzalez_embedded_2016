/** \mainpage conv
 *
 * \section intro_sec Introduction
 *
 * This application takes an image, applies a kernel on it and outputs a new image.
 *
 * \section usage_sec Usage
 *
 * <pre>
 * Usage: conv [options]
 * Options:
 *   -i INPUT_IMAGE     Input image to be processed.
 *   -o OUTPUT_IMAGE    Name for the output image generated.
 *   -k KERNEL_NUMBER   Kernel to execute: 1 = blur, 2 = sharpen.
 *   -a                 Print the information of the author of this program and exit.
 *   -h                 Print this message and exit.
 * </pre>
 *
 * \section kernels_sec Kernels
 *
 * Blur kernel is implemented as: {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625}\n
 * Sharpen kernel is implemented as: {0, -1, 0, -1, 5, -1, 0, -1, 0}
 *
 * \section examples_sec Examples
 *
 * Origial:
 * \image html ../lenna.jpg
 *
 * Blurred:
 * \image html ../lenna_blur.png
 *
 * Sharpened:
 * \image html ../lenna_sharpen.png
 *
 */
