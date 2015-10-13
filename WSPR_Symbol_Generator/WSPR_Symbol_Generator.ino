// WSPR_symbol_generator input coded, output on monitor
// based on code from Martin Nawrath, Acedemy of Media Arts, Cologne
// enter your call signe, locator and power dB *0 or *7 only
// open monitor and hit ENTER

const char SyncVec[162] = {
  1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,
  1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,
  0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,1,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,
  0,0,0,1,1,0,1,0,1,1,0,0,0,1,1,0,0,0
};

unsigned long n1;    // encoded callsign
unsigned long m1;    // encodes locator

byte c[11];                // encoded message
byte sym[170];             // symbol table 162
byte symt[170];            // symbol table temp

// put your data here
char call[] = " M6KWH";    // default values, 6 chars. 3rd numeric
char locator[] = "IO92";   // default value 4 chars
byte power = 20;           // default value 2 numberic

int ii, bb;

void setup()
{
  Serial.begin(9600);        // connect to the serial port
}

void loop()
{
  while(Serial.available() == 0);
  Serial.println("WSPR beacon");
  Serial.flush();
    
  encode_call();
  
  Serial.print("Call: ");
  Serial.println(call);

  encode_locator();
  
  Serial.print("Locator: ");
  Serial.println(locator);
  
  Serial.print("Power: ");
  Serial.println(power);
  
  encode_conv();

  Serial.println("");

  for (bb=0;bb<162 ;bb++)
  {
    Serial.print(symt[bb],DEC);
    Serial.print(",");
    if ( (bb+1) %32 == 0) Serial.println("");
  }
  Serial.println("");
  Serial.println("");

  interleave_sync();

  for (bb=0;bb<162 ;bb++)
  {
    Serial.print(sym[bb],DEC);
    Serial.print(",");
    if ((bb+1) %32 == 0) Serial.println("");
  }
  Serial.println("");
  
  while(Serial.available() > 0) Serial.read();
}


// encode sequence
void encode() 
{
  encode_call();
  encode_locator();
  encode_conv();
  interleave_sync();
};

// normalize characters 0..9 A..Z Space in order 0..36
char chr_normf(char bc ) 
{
  char cc=36;
  
  if (bc >= '0' && bc <= '9') cc=bc-'0';
  if (bc >= 'A' && bc <= 'Z') cc=bc-'A'+10;
  if (bc == ' ' ) cc=36;

  return(cc);
}

// encode call sign
void encode_call()
{
  unsigned long t1;

  n1=chr_normf(call[0]);
  n1=n1*36+chr_normf(call[1]);
  n1=n1*10+chr_normf(call[2]);
  n1=n1*27+chr_normf(call[3])-10;
  n1=n1*27+chr_normf(call[4])-10;
  n1=n1*27+chr_normf(call[5])-10;

  // merge coded callsign into message array c[]
  t1=n1;
  c[0]= t1 >> 20;
  t1=n1;
  c[1]= t1 >> 12;
  t1=n1;
  c[2]= t1 >> 4;
  t1=n1;
  c[3]= t1 << 4;
}

// encode locator
void encode_locator()
{
  unsigned long t1;
  
  // coding of locator
  m1=179-10*(chr_normf(locator[0])-10)-chr_normf(locator[2]);
  m1=m1*180+10*(chr_normf(locator[1])-10)+chr_normf(locator[3]);
  m1=m1*128+power+64;

  // merge coded locator and power into message array c[]
  t1=m1;
  c[3]= c[3] + ( 0x0f & t1 >> 18);
  t1=m1;
  c[4]= t1 >> 10;
  t1=m1;
  c[5]= t1 >> 2;
  t1=m1;
  c[6]= t1 << 6;
}

void encode_conv()
{
  int bc=0;
  int cnt=0;
  int cc;
  unsigned long sh1=0;

  cc=c[0];

  for (int i=0; i < 81;i++) 
  {
    if (i % 8 == 0 ) 
    {
      cc=c[bc];
      bc++;
    }
    if (cc & 0x80) sh1=sh1 | 1;

    symt[cnt++]=parity(sh1 & 0xF2D05351);
    symt[cnt++]=parity(sh1 & 0xE4613C47);

    cc=cc << 1;
    sh1=sh1 << 1;
  }
}

// calculate parity
byte parity(unsigned long li)
{
  byte po = 0;
  while(li != 0)
  {
    po++;
    li&= (li-1);
  }
  return (po & 1);
}

// interleave reorder the 162 data bits and and merge table with the sync vector
void interleave_sync()
{
  int ii,ij,b2,bis,ip;
  ip=0;

  for (ii=0;ii<=255;ii++) 
  {
    bis=1;
    ij=0;
    
    for (b2=0;b2 < 8 ;b2++) 
    {
      if (ii & bis) ij= ij | (0x80 >> b2);
      bis=bis << 1;
    }
    
    if (ij < 162 ) 
    {
      sym[ij]= SyncVec[ij] +2*symt[ip];
      ip++;
    }
  }
}

