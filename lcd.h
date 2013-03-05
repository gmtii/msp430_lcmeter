
static const unsigned long dv[] = {             // Base 10 digit weights
      10000000,                                 // 8 digit maximum count
       1000000,                                 //
        100000,                                 //
         10000,                                 //
          1000,                                 //
           100,                                 //
            10,                                 //
             1,                                 //
             0                                  //
};

typedef enum {
    lcd_command = 0,        // Array of one or more commands
    lcd_data = 1,           // Array of one or more bytes of data
    lcd_data_repeat = 2     // One byte of data repeated
} lcd_cmd_type;

void lcd_send(const unsigned char *cmd, unsigned len, const lcd_cmd_type type);
void lcd_home(void);
void lcd_pos(unsigned char x, unsigned char y);
void lcd_clear(unsigned char x);
void lcd_init(void);
void lcd_print(char *s, unsigned x, unsigned y);
void lcd_pd10(unsigned n, unsigned x, unsigned y);
void print_int(unsigned long n, unsigned char fila);

