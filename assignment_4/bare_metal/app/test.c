#include <stdint.h>

typedef volatile struct {
 uint32_t DR;
 uint32_t RSR_ECR;
 uint8_t reserved1[0x10];
 const uint32_t FR;
 uint8_t reserved2[0x4];
 uint32_t LPR;
 uint32_t IBRD;
 uint32_t FBRD;
 uint32_t LCR_H;
 uint32_t CR;
 uint32_t IFLS;
 uint32_t IMSC;
 const uint32_t RIS;
 const uint32_t MIS;
 uint32_t ICR;
 uint32_t DMACR;
} pl011_T;

enum {
 RXFE = 0x10,
 TXFF = 0x20,
};

pl011_T* const UART0 = (pl011_T*)0x101f1000;
//pl011_T* const UART1 = (pl011_T*)0x101f2000;
//pl011_T* const UART2 = (pl011_T*)0x101f3000;

// Print a single character to the UART
static void uart_putc(pl011_T* uart, const char c) {
 while (uart->FR & TXFF);
 uart->DR = c;
}

// Print a string to the UART
static void uart_puts(pl011_T* uart, const char* s) {
 int i = 0;
 while (*(s+i) != '\0') {
  uart_putc(uart, *(s+i));
  i++;
 }
}

// Capture a single character from the UART
static void uart_getc(pl011_T* uart, char* c) {
 if ((uart->FR & RXFE) == 0) {
  while (uart->FR & TXFF);
  *c = uart->DR;
  uart->DR = *c;
 } else {
  *c = '\0';
 }
}

// Capture a string from the UART
static void uart_gets(pl011_T* uart, char* s, int max_length) {
 char c = '\0';
 int i = 0;
 while (c != '\r' && i < max_length) {
  uart_getc(uart, &c);
  if (c != '\0' && c != '\r') {
   *(s+i) = c;
   i++;
  }
 }
 *(s+i) = '\0';
 uart_putc(uart, '\n');
}

// Converts from string to integer
static int stoi(char* s, int* error) {
 *error = 0;
 int i = 0;
 int neg = 0;
 int res = 0;
 if (*s == '-') {
  neg = 1;
  i++;
 }
 while (*(s+i) != '\0') {
  if (*(s+i) >= '0' && *(s+i) <= '9') {
   res = res * 10 + (*(s+i) - '0');
  } else {
   res = 0;
   *error = 1;
   break;
  }
  i++;
 }
 if (neg) res = -res;
 if (i == 0) *error = 1;
 return res;
}

// Converts from integer to string
static void itos(int n, char* res) {
 if(n == 0) {
  *(res) = '0';
  *(res+1) = '\0';
  return;
 }
 int neg = n < 0;
 unsigned int un = neg ? -n : n;
 int i = 0;
 while (un != 0) {
  *(res+i) = un % 10 + '0';
  un = un / 10;
  i++;
 }
 if (neg) {
  *(res+i) = '-';
  i++;
 }
 *(res+i) = '\0';
 // Reverse string
 int t;
 for (t = 0; t < i/2; t++) {
  char c = *(res+t);
  *(res+t) = *(res+i-t-1);
  *(res+i-t-1) = c;
 }
}

// Calculates the operation (inputs and outputs are strings)
static int calculate(char* f, char* s, char* o, char* r) {
 int err;
 int a = stoi(f, &err);
 if (err) return 1;
 int b = stoi(s, &err);
 if (err) return 2;
 int res;
 if (*o == '+') {
  res = a+b;
 } else if (*o == '*') {
  res = a*b;
 } else {
  res = 0;
  return 3;
 }
 itos(res, r);
 return 0;
}

// main
void c_entry() {
 char* f = "fst_placeholder\0";
 char* o = "\0";
 char* s = "sec_placeholder\0";
 char* r = "res_placeholder\0res_placeholder\0";
 int max_length = 16;
 int err = 1;

 while (1) {
  uart_puts(UART0, "Type the first operand: ");
  uart_gets(UART0, f, max_length);
  uart_puts(UART0, "Type the operation (+ or *): ");
  uart_gets(UART0, o, 1);
  uart_puts(UART0, "Type the second operand: ");
  uart_gets(UART0, s, max_length);
  err = calculate(f, s, o, r);
  if (err) {
   uart_puts(UART0, "ERROR: ");
   uart_putc(UART0, err+'0');
   uart_puts(UART0, "\n\n");
   continue;
  }
  uart_puts(UART0, "Result = ");
  uart_puts(UART0, r);
  //TEST
  uart_puts(UART0, "\n");
  uart_puts(UART0, f);
  uart_puts(UART0, " ");
  uart_puts(UART0, o);
  uart_puts(UART0, " ");
  uart_puts(UART0, s);
  uart_puts(UART0, " = ");
  uart_puts(UART0, r);
  // END TEST
  uart_puts(UART0, "\n\n");
 }
}

