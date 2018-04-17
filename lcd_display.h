#ifndef LCD_H
#define	LCD_H

void lcd_cmd(char command); //general lcd commands (not printing to lcd though)
void lcd_setCursor(char x, char y);//set location on lcd (x<8, y<2)
void lcd_printChar(char myChar); 
void lcd_printStr(const char *s);
void lcd_init(char contrast); 

#endif	/* LCD_H */


