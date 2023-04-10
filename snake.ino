#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define BitSet(arg,bit) ((arg) |= ((B10000)>>bit))
#define BitClr(arg,bit) ((arg) &= ~((B10000)>>bit)) 

struct button{
  unsigned char value;
  const unsigned char pin;
};

struct coord{
  char x, y;
};

struct coordNode{
  unsigned char x, y;
  coordNode *next;
};

LiquidCrystal_I2C lcd(0x27, 16, 2);
button right = {0, 2}, down = {0, 3}, left = {0, 4}, up = {0, 5};
coordNode* snakeHead = NULL;
coordNode* snakeTail = NULL;
char snakeDir;
coord food;
coord customChars[8];

byte board[16][2][8] = {0};

void push(unsigned char x, unsigned char y)
{
  coordNode *aux = malloc(sizeof(coordNode));
  aux->x = x;
  aux->y = y;
  aux->next = NULL;
  if(snakeHead != NULL)
    snakeHead->next = aux;
  else
    snakeTail = aux;
  snakeHead = aux;
}

void pop()
{
  coordNode *aux = snakeTail;
  snakeTail = snakeTail->next;
  free(aux);
}

bool emptyChar(byte c[8])
{
  for (unsigned char i = 0; i < 8; i++)
    if (c[i])
      return false;
  return true;
}

void pixelON(unsigned char x, unsigned char y)
{
  unsigned char i;
  BitSet(board[x/5][y/8][y%8], x%5);
  for (i = 0; i < 8; i++)
    if (customChars[i].x == x/5 && customChars[i].y == y/8)
      break;
  if (i == 8)
  {
    for (i = 0; i < 8; i++)
      if (customChars[i].x == -1)
      {
        customChars[i].x = x/5;
        customChars[i].y = y/8;
        break;
      }
  }
  lcd.createChar(i, board[x/5][y/8]);
  lcd.setCursor(x/5, y/8);
  lcd.write(i);
}

void pixelOFF(unsigned char x, unsigned char y)
{
  unsigned char i;
  BitClr(board[x/5][y/8][y%8], x%5);
  if (emptyChar(board[x/5][y/8]))
  {
    lcd.setCursor(x/5, y/8);
    lcd.print(' ');
    for (i = 0; i < 8; i++)
      if (customChars[i].x == x/5 && customChars[i].y == y/8)
        customChars[i].x = -1;
  }else
  {
    for (i = 0; i < 8; i++)
      if (customChars[i].x == x/5 && customChars[i].y == y/8)
        break;
    if (i == 8)
    {
      for (i = 0; i < 8; i++)
        if (customChars[i].x == -1)
        {
          customChars[i].x = x/5;
          customChars[i].y = y/8;
          break;
        }
    }
    lcd.createChar(i, board[x/5][y/8]);
    lcd.setCursor(x/5, y/8);
    lcd.write(i);
  }
}

void initSnake()
{
  snakeDir = 'E';
  push(39, 8);
  push(40, 8);
  push(41, 8);
  pixelON(39, 8);
  pixelON(40, 8);
  pixelON(41, 8);
}

void newFood()
{
  food.x = random(80);
  food.y = random(16);
  pixelON(food.x, food.y);
}

void hello()
{
  lcd.setCursor(1, 0);
  lcd.print("FUNDACAO ASIMO");
  delay(1000);
}

bool isButton(button& b)
{
  return !digitalRead(b.pin);
}

void keyboardRead()
{
  if (snakeDir == 'N' || snakeDir == 'S')
  {
    if(isButton(right))
      snakeDir = 'E';
    if(isButton(left))
      snakeDir = 'W';
  }
  if (snakeDir == 'E' || snakeDir == 'W')
  {
    if(isButton(up))
      snakeDir = 'N';
    if(isButton(down))
      snakeDir = 'S';
  }
}

void restart()
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(1, 1);
  lcd.print("precione algo");
  while(snakeTail != NULL)
    pop();
  snakeHead = snakeTail;
  memset(customChars, -1, sizeof(customChars));
  memset(board, 0, sizeof(board));
  while(!(isButton(up) || isButton(down) || isButton(right) || isButton(left)));    
  lcd.clear();
  initSnake();
  newFood();
}

void setup()
{
  unsigned char i;

  lcd.init();
  lcd.backlight();
  hello();
  randomSeed(analogRead(0));
  for (i = 0; i < 8; i++)
    customChars[i] = {-1, -1};
  for (i = 2; i <= 5; i++)
    pinMode(i, INPUT_PULLUP);
  
  lcd.clear();
  initSnake();
  newFood();
}


void loop()
{
  unsigned char x = snakeHead->x, y = snakeHead->y;
  keyboardRead();
  if(snakeDir == 'N' && y >= 0)
    y--;
  if(snakeDir == 'S' && y < 16)
    y++;
  if(snakeDir == 'W' && x >= 0)
    x--;
  if(snakeDir == 'E' && x < 80)
    x++;

  if (x < 80 && x >= 0 && y < 16 && y >= 0)
  {
    push(x, y);
    pixelON(snakeHead->x, snakeHead->y);
    if (x == food.x && y == food.y)      
      newFood();
    else
    {
      pixelOFF(snakeTail->x, snakeTail->y);
      pop();
    }
  }else
  {
    restart();
  }
  delay(150);
}
