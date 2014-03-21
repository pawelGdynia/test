//---------------------------
//  weze.cpp - przetwarzanie
//---------------------------

#pragma hdrstop
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

// w kt�rych tabelach jest przechowywany opis obiektu
#define TAB_GRUNT   1
#define TAB_ROSLINA 2
#define TAB_ZWIERZ  3

#define MAX_SIZE 20
typedef struct
  {
  short pm1_tab; // TAB_*: w kt�rej tabeli obiekt�w znajduje si� opis
  short pm1_poz; // pozycja w tabeli dla wskazanego typu
  } PUNKT_MAPY1;

// dane 1 punktu na mapie
typedef struct
  {
  PUNKT_MAPY1 pm_grunt;
  PUNKT_MAPY1 pm_roslina;
  PUNKT_MAPY1 pm_zwierz;
  short x; // do pomocy przy debuggowaniu
  short y;
  } PUNKT_MAPY;

// W�a�ciwo�ci WSP�LNE
typedef struct
  {
  short x;  // wsp�rz�dna x na mapie (g�owa dla zwierza)
  short y;  // wsp�rz�dna y na mapie
  short po; // czy po przetworzeniu
  } OBIEKTINFO_COMMON;

//=== GRUNTY ===============================================================
typedef struct // info wsp�lne dla wszystkich wyst�pie�
  {
  short dg_blok; // miejsce niedost�pne - ani ro�lin ani zwierz�t
  short dg_dead; // miejsce martwe - bez ro�lin, ale dost�pne dla zwierz�t
  } DEF_GRUNT;

typedef struct // info dla jednego punktu
  {
  short g_def;              // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON g_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu: brak
  } OBIEKTINFO_GRUNT;

//== RO�LINY ===============================================================
typedef struct // info wsp�lne dla wszystkich wyst�pie�
  {
  short dr_czasWzrostu; // max poziom, ile okres�w na pe�ny wzrost od zera
  short dr_kalorie;     // ile punkt�w zyskuje zjadaj�cy
  } DEF_ROSLINA;

typedef struct
  {
  short r_def; // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON r_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short r_poziom; // bie�acy poziom wzrostu - max to defr_czasWzrostu
  } OBIEKTINFO_ROSLINA;

//=== ZWIERZ�TA ============================================================
typedef struct // info wsp�lne dla wszystkich wyst�pie�
  {
  short dz_roslinozerca;
  short dz_drapieznik;
  short dz_maxZapas;   // ile zasobow (trawy) mo�e przechowa�
  short dz_kalorie;    // ile punktow daje zjedzenie go
  short dz_maxSize;    // do ilu modu��w mo�e rosn��
  short dz_utrata;     // ile zasob�w ubywa dla nowego modu�u lub potomka
  short dz_zasieg;     // jak daleko widzi aby analizowa� sytuacj� (0,1,2)
  } DEF_ZWIERZ;

static DEF_ZWIERZ defZwierz[] = {
// Rosl Drap Zapas Kalorie  maxSize Utrata Zasieg
  {1,   0,     10,      20,      1,     5,    3},
  {0,   1,     90,      10,     10,     5,    3},
  };
#define ILE_DEFZWIERZ (sizeof(defZwierz)/sizeof(defZwierz[0]))

typedef struct
  {
  short z_def; // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON z_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short z_zapas;  // bie�acy poziom zapasu �ywno�ci
  short z_x[MAX_SIZE+1];// wsp�rz�dne wszystkich segment�w
  short z_y[MAX_SIZE+1];
  short z_size;
  short z_id; // unikalny numer
  } OBIEKTINFO_ZWIERZ;




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_output
//!  Procedury wy�wietlania na ekranie
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static HANDLE hStdOut;
//---------------------------------------------------------------------------
//! Ustaw kolor tekstu - kolor zdefiniowany w windows.h
void SetTekstKolor(short kolor)
{
  SetConsoleTextAttribute(hStdOut, kolor);
} // SetTekstKolor

//---------------------------------------------------------------------------
//! Drukuj tekst wg bie��cego atrybutu
void PutZnak(char* znak)
{
  unsigned long count;
  WriteConsole(hStdOut, znak, 1, &count, NULL);
} // PutZnak

//---------------------------------------------------------------------------
void SetKursorPoz(short x, short y)
{
  COORD coord = {0,0};
  coord.X = x;
  coord.Y = y;
  SetConsoleCursorPosition(hStdOut, coord);
} // SetKursorPoz

//---------------------------------------------------------------------------
void PrzygotujEkran(void)
{
  clrscr();
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  SetTekstKolor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
} // PrzygotujEkran

//@} weze_output




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_przetwarzanie
//!  Alogorytmy przetwarzania obiekt�w i mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static short xSize = 50;
static short ySize = 20;

static PUNKT_MAPY* mapa;
static DEF_GRUNT defGrunt[] = {
  {0,0}, // 0, ziemia rolna
  {1,1}, // 1, ocean
  {0,1}  // 2, pustynia
  };
#define ILE_DEFGRUNT (sizeof(defGrunt)/sizeof(defGrunt[0]))
#define G_ROLNA  0
#define G_OCEAN  1
#define G_PUSTKA 2
static DEF_ROSLINA defRoslina[] = {
  {9, 2},
  };
#define ILE_DEFROSLINA (sizeof(defRoslina)/sizeof(defRoslina[0]))

/* funkcje do zrobienia:
- wype�nij map� danymi z pliku txt
*/

static long ileGen=0; // ile przebieg�w
static long martwe=0; // ile odesz�o
static OBIEKTINFO_GRUNT*   listaGrunt;
static OBIEKTINFO_ROSLINA* listaRoslin;
static OBIEKTINFO_ZWIERZ*  listaZwierz;
static short ileGrunt =0;
static short ileRoslin=0;
static short ileZwierz=0;
static short maxZwierzId=0;
static char  kierunek = ' '; //!< kierunek wklepany przez operatora
static short selZwierz = 1;   //!< wybrany jako pod�wietlony
static short debug=0;
short WybranyKierunek(void);
short PunktWlasnyZwierz(OBIEKTINFO_ZWIERZ* ptrZ1, short x, short y);
//---------------------------------------------------------------------------
//! Zwraca adres punktu na mapie, niezale�nie od wewn�trznej organizacji
PUNKT_MAPY* PtrPunktMapy(short x, short y)
{
  return &(mapa[(x*ySize)+y]);
} // PtrPunktMapy

//---------------------------------------------------------------------------
//! Przygotuj 3 tabele na opisy obiekt�w
void PusteTabele(void)
{
  ileGrunt  = 0;
  ileRoslin = 0;
  ileZwierz = 0;
  memset(listaGrunt,  0, sizeof(OBIEKTINFO_GRUNT)  * (xSize * ySize +1));
  memset(listaRoslin, 0, sizeof(OBIEKTINFO_ROSLINA)* (xSize * ySize +1));
  memset(listaZwierz, 0, sizeof(OBIEKTINFO_ZWIERZ) * (xSize * ySize +1));
  martwe = 0;
} // PusteTabele

//---------------------------------------------------------------------------
//! Dopisz na mapie dane gruntu
void DodajGrunt(short x, short y, short id)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);
  if (ileGrunt >= xSize * ySize)
    {
    printf("E:DodajGrunt()");
    return; // nie dodawaj - nie ma miejsca
    }
  // info w samej mapie
  ptr->pm_grunt.pm1_tab = TAB_GRUNT;
  ptr->pm_grunt.pm1_poz = ileGrunt;

  // info w obiekcie (najbli�sze wolne miejsce)
  listaGrunt[ileGrunt].g_common.x = x; // og�lne
  listaGrunt[ileGrunt].g_common.y = y;
  listaGrunt[ileGrunt].g_common.po= 0; // nieprzetworzony
  // dok�adne info o obiekcie
  listaGrunt[ileGrunt].g_def = id;

  ileGrunt++;
} // DodajGrunt

//---------------------------------------------------------------------------
//! Dopisz na mapie dane ro�liny
void DodajRosline(short x, short y, short id)
{
  PUNKT_MAPY* ptrMapa;
  OBIEKTINFO_GRUNT* ptrG;
  DEF_GRUNT* defG;
  if (ileRoslin >= xSize * ySize)
    {
    printf("E:DodajRosline()");
    return; // nie dodawaj - nie ma miejsca
    }
  ptrMapa = PtrPunktMapy(x,y);
  // sprawd�, czy grunt si� nadaje
  ptrG = listaGrunt + (ptrMapa->pm_grunt.pm1_poz);
  defG = defGrunt + ptrG->g_def;
  if (defG->dg_blok != 0
    ||defG->dg_dead != 0)
    return; // miejsce si� nie nadaje dla ro�lin
  // info w samej mapie
  ptrMapa->pm_roslina.pm1_tab = TAB_ROSLINA;
  ptrMapa->pm_roslina.pm1_poz = ileRoslin;

  // info w obiekcie
  listaRoslin[ileRoslin].r_common.x = x;
  listaRoslin[ileRoslin].r_common.y = y;
  listaRoslin[ileRoslin].r_common.po= 0; // nieprzetworzony
  // dok�adne info o obiekcie
  listaRoslin[ileRoslin].r_def = id;
  // poziom wzrostu - ustaw maksymalny
  listaRoslin[ileRoslin].r_poziom = defRoslina[id].dr_czasWzrostu;

  ileRoslin++;
} // DodajRosline

//---------------------------------------------------------------------------
//! Dopisz na mapie dane zwierza
void DodajZwierz(short x, short y, short id)
{
  PUNKT_MAPY* ptrMapa;
  OBIEKTINFO_ZWIERZ* ptrZ;
  DEF_ZWIERZ* ptrDef;      // definicja
  OBIEKTINFO_GRUNT* ptrG;
  DEF_GRUNT* defG;

  if (ileZwierz >= xSize * ySize)
    {
    printf("E:DodajZwierz()");
    return; // nie dodawaj - nie ma miejsca
    }
  ptrMapa = PtrPunktMapy(x,y);
  ptrG = listaGrunt + (ptrMapa->pm_grunt.pm1_poz);
  defG = defGrunt + ptrG->g_def;
  if (defG->dg_blok != 0)
    return; // miejsce si� nie nadaje dla zwierz�t

  ptrZ = listaZwierz+ileZwierz;
  ptrDef = defZwierz + id;
  // info w samej mapie
  ptrMapa->pm_zwierz.pm1_tab = TAB_ZWIERZ;
  ptrMapa->pm_zwierz.pm1_poz = ileZwierz;

  // info w obiekcie
  ptrZ->z_common.x = x;
  ptrZ->z_common.y = y;
  ptrZ->z_common.po= 0; // nieprzetworzony
  // dok�adne info o obiekcie
  ptrZ->z_def = id;
  ptrZ->z_zapas = ptrDef->dz_maxZapas-2; // max -2

  // lista segment�w - dok�adnie jeden
  ptrZ->z_x[0] = x;
  ptrZ->z_y[0] = y;
  ptrZ->z_size = 1;

  maxZwierzId++;
  if (maxZwierzId > 9999)
    maxZwierzId = 1;
  ptrZ->z_id = maxZwierzId;
  ileZwierz++;
} // DodajZwierz

//---------------------------------------------------------------------------
//! Wype�nij struktury pierwsz� generacj� obiekt�w
void ZapelnijMapeFull(void)
{
  short x,y;
  PUNKT_MAPY* ptrMapa;

  // Pierwszy przebieg - grunty
  for (x=0; x<xSize; x++)
    for (y=0; y<ySize; y++)
      {
      ptrMapa = PtrPunktMapy(x,y);
      ptrMapa->x = x;
      ptrMapa->y = y;
      // grunty rolne - wsz�dzie uprawne (typ 0)
      DodajGrunt(x, y, G_ROLNA);
      }

  DodajZwierz(3, 1, 1);
  DodajZwierz(4, 1, 1);

  for (x=0; x<xSize; x++)
    for (y=0; y<ySize; y++)
      {
      ptrMapa = PtrPunktMapy(x,y);

      // ro�liny
      //if (y%2) // tylko kilka rz�dk�w ro�liny zwyk�ej (typ 0)
        DodajRosline(x, y, 0);
      if (x==y)
        DodajZwierz(x, y, 0); // gatunek "0"
      }
} // ZapelnijMapeFull

char mapa1[20][51] = {
{"000000000000000000000000000000000000000111111111110"},
{"011111122222222222222222100000001111111111111111100"},
{"010001111111111111111111110001111111111111111111111"},
{"011011112222222222222222210000111111111111111111111"},
{"011111111111111111111111110011111111111111111111111"}, // 5
{"011111112222222222222222210011111111111111111111111"},
{"001111111111111111111111110001111111111111111111111"},
{"000000000000122222222222111100111111111111111111111"},
{"000000000222222222222211100000111111111111111111111"},
{"000001111222222222222220000000011111111111111111111"}, // 10
{"011111111222222222222221100000000111111111111111111"},
{"011111111110000000000111111111111100000000000000001"},
{"011111111000000000000001111111111000000000000000001"},
{"011111110000000000000000111111111000000000000000001"},
{"011111111000000000000000000000010000000000000000001"}, // 15
{"011111111100000000000000000000111111111111111111111"},
{"001111111111111100000000011111111111111111111111111"},
{"000001111111111111111111111111111111111111111111111"},
{"000000011111111111111111111111111111111111111111111"},
{"000000001111111111111111111111111111111111111111111"}, // 20
};

//---------------------------------------------------------------------------
//! Zamie� 0,1,2 na rodzaj gruntu
char Znak2Grunt(char znak)
{
  switch (znak)
    {
    case '0':
      return G_OCEAN; // ocean

    case '1': // pustynia
      return G_PUSTKA;

    case '2':
      return G_ROLNA; // uprawna
    }
  return G_OCEAN; // warto�� domy�lna - ocean
} // Znak2Grunt

//---------------------------------------------------------------------------
//! Wype�nij map� - wg tabeli z pami�ci
void ZapelnijMapeTab(void)
{
  short x, y;
  char znak;
  PUNKT_MAPY* ptrMapa;
    
  for (x=0; x<xSize; x++)
    for (y=0; y<ySize; y++)
      {
      ptrMapa = PtrPunktMapy(x,y);
      ptrMapa->x = x;
      ptrMapa->y = y;
      // grunty rolne - wsz�dzie uprawne (typ 0)
      znak = Znak2Grunt(mapa1[y][x]);
      DodajGrunt(x, y, znak);
      }
  DodajZwierz(3, 1, 1);
} // ZapelnijMapeTab

//---------------------------------------------------------------------------
//! Wype�nij map� - z jednego z kilku �r�de�
void ZapelnijMape(void)
{
  //ZapelnijMapeFull();
  ZapelnijMapeTab();
} // ZapelnijMape

short losowe[24][4] = { // wszystkie mo�liwe kolejno�ci dla 4 element�w
  {1,2,3,4}, {1,2,4,3}, {1,3,2,4}, {1,3,4,2}, {1,4,2,3}, {1,4,3,2},
  {2,1,3,4}, {2,1,4,3}, {2,3,1,4}, {2,3,4,1}, {2,4,1,3}, {2,4,3,1},
  {3,1,2,4}, {3,1,4,2}, {3,2,1,4}, {3,2,4,1}, {3,4,1,2}, {3,4,2,1},
  {4,1,2,3}, {4,1,3,2}, {4,2,1,3}, {4,2,3,1}, {4,3,1,2}, {4,3,2,1}
  };
//---------------------------------------------------------------------------
//! Ustaw losowo liczby 1-4
void UstalRandom4(short* num4)
{
  short a;
  a = random(24); // wybierz - kt�ra kolejno�� z 24 zostanie u�yta
//  a = 1;
  num4[0] = losowe[a][0];
  num4[1] = losowe[a][1];
  num4[2] = losowe[a][2];
  num4[3] = losowe[a][3];
} // UstalRandom4

//---------------------------------------------------------------------------
//! Sprawd� czy zadane wsp�rz�dne mieszcz� si� na mapie
short CzyPunktZakres(short x, short y)
{
  if (x >= 0
    &&y >= 0
    &&x < xSize
    &&y < ySize)
    return 1; // ok
  return 0; // poza zakresem - tam nic nie ma!
} // CzyPunktZakres

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wsp�rz�dnych wyst�puje obiekt - ro�lina
short CzyMapaRoslina(short x, short y)
{
  PUNKT_MAPY* ptrMapa;
  ptrMapa = PtrPunktMapy(x,y);

  if (ptrMapa->pm_roslina.pm1_tab == 0)
    return 0; // nie ma tam ro�liny

  return 1; // jest
} // CzyMapaRoslina

//---------------------------------------------------------------------------
//! Kt�r� pozycj� w tabeli zajmuje ro�lina?
short GetPozRoslina(short x, short y)
{
  PUNKT_MAPY* ptrMapa;
  ptrMapa = PtrPunktMapy(x,y);

  return ptrMapa->pm_roslina.pm1_poz;
} // GetPozRoslina

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wsp�rz�dnych zwierz zwyk�y (1) czy drapie�nik (2)
short CzyMapaZwierz(short x, short y)
{
  PUNKT_MAPY* ptrMapa;
  DEF_ZWIERZ* ptrDef;      // definicja
  short poz;
  ptrMapa = PtrPunktMapy(x,y);

  if (ptrMapa->pm_zwierz.pm1_tab == 0)
    return 0; // nie ma tam nikogo
  poz = ptrMapa->pm_zwierz.pm1_poz;
  if (listaZwierz[poz].z_def<0)
    return 0; // jest ale martwy
  ptrDef = defZwierz + listaZwierz[poz].z_def;
  if (ptrDef->dz_drapieznik > 0)
    return 2; // jest drapie�nik

  return 1; // jest ro�lino�erca
} // CzyMapaZwierz

//---------------------------------------------------------------------------
//! Kt�r� pozycj� w tabeli zajmuje zwierz?
short GetPozZwierz(short x, short y)
{
  PUNKT_MAPY* ptrMapa;
  ptrMapa = PtrPunktMapy(x,y);

  return ptrMapa->pm_zwierz.pm1_poz;
} // GetPozZwierz

//---------------------------------------------------------------------------
//! Kt�r� pozycj� w tabeli zajmuje zwierz?
short CzyPunktLadowy(short x, short y)
{
  PUNKT_MAPY* ptrMapa;
  OBIEKTINFO_GRUNT* ptrG;
  DEF_GRUNT* ptrDef;

  ptrMapa = PtrPunktMapy(x,y);
  ptrG    = listaGrunt + ptrMapa->pm_grunt.pm1_poz;
  ptrDef  = defGrunt + ptrG->g_def;
  if (ptrDef->dg_blok)
    return 0;
  else
    return 1;  
} // CzyPunktLadowy

//---------------------------------------------------------------------------
//! Uniwersalna procedura sprawdzania czy ro�lina jadalna
short RoslinaJadalna(short x, short y)
{
  short poz, ktoraDef;

  OBIEKTINFO_ROSLINA* ptrRoslina;
  DEF_ROSLINA* ptrDef;

  if (CzyMapaRoslina(x,y)==0)
    return 0; // nie ma tam ro�liny!

  poz = GetPozRoslina(x,y); // kt�ra ro�lina z listy
  ptrRoslina = listaRoslin+poz;
  ptrDef     = defRoslina + ptrRoslina->r_def;

  //-----------
  if (ptrRoslina->r_poziom == ptrDef->dr_czasWzrostu)
    return 1; // tylko maksymalne s� jadalne

  return 0;
} // RoslinaJadalna
                       // 0   1  2  3  4   5  6  7  8   9 10 11 12
static short waz_x[]   = {0,  0, 1, 0,-1, -1, 1, 1,-1,  0, 2, 0,-2};
static short waz_y[]   = {0, -1, 0, 1, 0, -1,-1, 1, 1, -2, 0, 2, 0};
static short waz_posr1[]={0,  1, 2, 3, 4,  1, 2, 3, 4,  1, 2, 3, 4};
static short waz_posr2[]={0,  0, 0, 0, 0,  4, 1, 2, 3,  0, 0, 0, 0}; // droga alternatywn
static short ilePkt[]   = {1,5,9,13}; // ile punkt�w sprawdza� zale�nie od zasi�gu

//---------------------------------------------------------------------------
// Ustal kolejno�� sprawdzania s�siednich punkt�w
void Random2Ktore(short* kolej, short* ktore)
{
  short a;
  ktore[0] = 0;
  for (a=0; a<4; a++)
    {
    ktore[a+1+0] = kolej[a]+0; // wymieszaj w ramach czw�rki 1-4
    ktore[a+1+4] = kolej[a]+4; // wymieszaj w ramach czw�rki 5-8
    ktore[a+1+8] = kolej[a]+8; // wymieszaj w ramach czw�rki 9-12
    }
} // Random2Ktore

//---------------------------------------------------------------------------
//! Wykonaj sprawdzanie dla podanego punktu s�siedniego
/*short SprawdzPunktSasiedni(short x1, short y1)
{
  return 0;
} // SprawdzPunktSasiedni
*/
//---------------------------------------------------------------------------
//! Wybierz z s�siedztwa miejsce do zjedzenia, wynik=1 oznacza �e wybrano
short WybierzFood(short x1, short y1, short* destX, short* destY, DEF_ZWIERZ* def)
{
  short a;
  short kolej[4];  // wynik z random
  short ktore[13]; // kolejno�� sprawdzania
  short tmpX; // tymczasowe
  short tmpY;

  short bestX = -1; // ruch do wykonania w kierunku jedzenia
  short bestY = -1;

  short wolneX = x1; // proponowany punkt gdy nie ma lepszej propozycji
  short wolneY = y1;

  short zasieg = def->dz_zasieg;
  short ile = ilePkt[zasieg]; // ile punkt�w z tabeli sprawdza�

  UstalRandom4(kolej); // => kolej[4]
  Random2Ktore(kolej, ktore); // ustal kolejno�� w ktore[] wed�ug kolej[4]

  // w losowej kolejno�ci - sprawd� punkty s�siednie
  for (a=0; a<ile; a++)
    {
    short ruch = ktore[a];
    tmpX = x1 + waz_x[ ruch ];
    tmpY = y1 + waz_y[ ruch ];
    if (CzyPunktZakres(tmpX, tmpY)==0) // punkt wyszed� poza map�, pomi� go
      continue;
    if (CzyPunktLadowy(tmpX, tmpY)==0) // czy mozna wej�� na ten punkt
      continue;

    if (a < 5 && CzyMapaZwierz(tmpX, tmpY)==0) // gdy miejsce wolne
      {
      wolneX = tmpX;
      wolneY = tmpY;
      }
    if (def->dz_drapieznik) // jestem drapie�nikiem
      {
      if (CzyMapaZwierz(tmpX, tmpY)==1) // ro�lino�erca (1) w punkcie docelowym
        {
        if (a < 5) // jest blisko, mo�na go zje��
          {
          *destX = tmpX;
          *destY = tmpY;
          return 2; // zjedz go!
          }
        else // jest dalej, id� w jego kierunku
          {
          if (bestX == -1) // nie by�o wcze�niej takiej sytuacji
            {
            short x,y;

            // zamie� na punkt s�siaduj�ce z punktem wyj�ciowym
            x = x1 + waz_x [ waz_posr1[ruch]  ];
            y = y1 + waz_y [ waz_posr1[ruch]  ];
            if (CzyPunktZakres(x, y)
              &&CzyMapaZwierz(x, y) == 0) // jest wolne, mo�na u�y�
              {
              bestX = x;
              bestY = y;
              }
            else // spr�buj drogi alternatywnej
              {
              if (waz_posr2[a] != 0) // jest zdefiniowana droga alternatywna
                {
                x = x1 + waz_x [ waz_posr2[ruch] ];
                y = y1 + waz_y [ waz_posr2[ruch] ];
                if (CzyPunktZakres(x, y)
                  &&CzyMapaZwierz(x, y) == 0) // jest wolne, mo�na u�y�
                  {
                  bestX = x;
                  bestY = y;
                  }
                }
              }
            }
          }
        }
      }

    if (def->dz_roslinozerca) // jestem ro�lino�erc�
      if (CzyMapaZwierz(tmpX, tmpY)==0) // nie ma tam zwierza
        {
        if (RoslinaJadalna(tmpX, tmpY))
          {
          if (a < 5)
            {
            *destX = tmpX;
            *destY = tmpY;
            return 1; // zjedz ro�lin�
            }
          else // jest dalej, id� w jego kierunku
            {
            if (bestX == -1) // nie by�o wcze�niej takiej sytuacji
              {
              short x,y;

              // zamie� na punkt s�siaduj�ce z punktem wyj�ciowym
              x = x1 + waz_x [ waz_posr1[ruch]  ];
              y = y1 + waz_y [ waz_posr1[ruch]  ];
              if (CzyPunktZakres(x, y)
                &&CzyMapaZwierz(x, y) == 0) // jest wolne, mo�na u�y�
                {
                bestX = x;
                bestY = y;
                }
              else // spr�buj drogi alternatywnej
                {
                if (waz_posr2[a] != 0) // jest zdefiniowana droga alternatywna
                  {
                  x = x1 + waz_x [ waz_posr2[ruch] ];
                  y = y1 + waz_y [ waz_posr2[ruch] ];
                  if (CzyPunktZakres(x, y)
                    &&CzyMapaZwierz(x, y) == 0) // jest wolne, mo�na u�y�
                    {
                    bestX = x;
                    bestY = y;
                    }
                  }
                }
              }
            }
          }
        }
    }

  // przyjmij inny punkt w otoczeniu - wolny
  if (bestX != -1)
    {
    *destX = bestX;
    *destY = bestY;
    }
  else
    {
    *destX = wolneX;
    *destY = wolneY;
    }
  if (x1 == *destX
    ||y1 == *destY)
    ; // ok, to punkt s�siedni
  else
    printf("To nie jest s�siedni punkt!");
  return 0; // nie ma nic jadalnego w okolicy
} // WybierzFood

//---------------------------------------------------------------------------
//! Wybierz z s�siedztwa miejsce do umieszczenia noworodka (wynik=1 to zgoda)
short WybierzDlaNowego(short x1, short y1, short* destX, short* destY)
{
  short kolej[4];
  short a;


  UstalRandom4(kolej);
  // w losowej kolejno�ci - punkty s�siednie
  for (a=0; a<4; a++)
    {
    switch (kolej[a])
      {
      case 1: // na g�rze
        *destX = x1;
        *destY = y1-1;
        break;

      case 2: // z prawej
        *destX = x1+1;
        *destY = y1;
        break;

      case 3: // na dole
        *destX = x1;
        *destY = y1+1;
        break;

      case 4: // z lewej
        *destX = x1-1;
        *destY = y1;
        break;
      }
    if (CzyPunktZakres(*destX, *destY))
      if (CzyMapaZwierz(*destX, *destY)==0) // miejsce jest wolne
        return 1; // wybierz ten punkt
    }

  return 0; // nie mam miejsca
} // WybierzDlaNowego

//---------------------------------------------------------------------------
//! Ustal pozycj� na podstawie id
short Id2PozZwierz(short id)
{
  short a;
  for (a=0; a<ileZwierz; a++)
    if (listaZwierz[a].z_id == id)
      return a; // pozycja pasuj�cego
  return -1; // gdy nie znaleziono
} // Id2PozZwierz

//---------------------------------------------------------------------------
// Przetw�rz ro�lin� z listy
void ProcessRoslina(short poz)
{
  OBIEKTINFO_ROSLINA* ptrRoslina;
  DEF_ROSLINA* ptrDef;

  ptrRoslina = listaRoslin+poz;
  ptrDef     = defRoslina + ptrRoslina->r_def;

  if (ptrRoslina->r_poziom < ptrDef->dr_czasWzrostu)
    ptrRoslina->r_poziom++; // wzrost +1
} // ProcessRoslina

//---------------------------------------------------------------------------
//! Przemie�� w nowe miejsce, z dodaniem nowego segmentu lub nie
void PrzesunZwierz(short poz, short srcX, short srcY, short dstX, short dstY, short plus)
{
  short a;
  PUNKT_MAPY* ptrMapa=NULL;
  OBIEKTINFO_ZWIERZ* ptrZ;

  ptrZ = listaZwierz+poz;
  // w tabeli zwierz zmie� wsp�rz�dn� g�owy na mapie
  ptrZ->z_common.x = dstX;
  ptrZ->z_common.y = dstY;
  //===== MAPA: wyzeruj punkty sprzed zmiany
  for (a=0; a<ptrZ->z_size; a++)
    {
    ptrMapa = PtrPunktMapy(ptrZ->z_x[a], ptrZ->z_y[a]);
    ptrMapa->pm_zwierz.pm1_tab = 0;
    ptrMapa->pm_zwierz.pm1_poz = 0;
    }
  //===== LISTA SEGMENT�W: przesu� na li�cie
  for (a=ptrZ->z_size; a>=0; a--) // od ostatniego w d�
    {
    if (a==0) // pierwszy na li�cie - wstaw wsp�rz�dne docelowe g�owy
      {
      ptrZ->z_x[a] = dstX;
      ptrZ->z_y[a] = dstY;
      }
    else // pozosta�e - wstaw wsp�rz�dna tego co by� wy�ej
      {
      ptrZ->z_x[a] = ptrZ->z_x[a-1];
      ptrZ->z_y[a] = ptrZ->z_y[a-1];
      }
    } // for

  if (plus) // zwi�kszania rozmiaru - NIE kasuj ostatniego segmentu
    ptrZ->z_size++; // zwi�ksz rozmiar
  else
    {
    // wyzeruj ostatni
    ptrZ->z_x[ptrZ->z_size] = 0;
    ptrZ->z_y[ptrZ->z_size] = 0;
    }
  //===== przepisz dane do mapy
  for (a=0; a<ptrZ->z_size; a++)
    {
    ptrMapa = PtrPunktMapy(ptrZ->z_x[a], ptrZ->z_y[a]);
    ptrMapa->pm_zwierz.pm1_tab = TAB_ZWIERZ;
    ptrMapa->pm_zwierz.pm1_poz = poz;
    }
} // PrzesunZwierz

//---------------------------------------------------------------------------
//! Oznakuj martwego zwierza na li�cie, z mapy usu� ca�kiem
void UsunMartwego(short poz)
{
  short a;
  OBIEKTINFO_ZWIERZ* ptrZ; // zwierz przetwarzany
  PUNKT_MAPY* ptrDst=NULL;

  ptrZ = listaZwierz+poz;
  // 1.usu� dane z mapy
  for (a=0; a<ptrZ->z_size; a++)
    {
    ptrDst = PtrPunktMapy(ptrZ->z_x[a], ptrZ->z_y[a]);
    ptrDst->pm_zwierz.pm1_tab = 0;
    ptrDst->pm_zwierz.pm1_poz = 0;
    }

  // 2.oznakuj w tabeli jako martwy
  listaZwierz[poz].z_def = -1;
} // UsunMartwego

//---------------------------------------------------------------------------
//! Sprawd� czy w�owe g�owy s� na miejscu
void TestMapy(void)
{
  short a, max, x,y;
  PUNKT_MAPY* ptrMapa=NULL;

  max = ileZwierz; // ustal ilo�� przed - nowo dodane nie b�d� uwzgl�dnione
  for (a=0; a<max; a++)
    {
    OBIEKTINFO_ZWIERZ* ptrZ1; // zwierz przetwarzany
    ptrZ1 = listaZwierz+a;
    if (ptrZ1->z_def != -1) // pomijaj usuni�te
      {
      x = ptrZ1->z_common.x;
      y = ptrZ1->z_common.y;
      ptrMapa = PtrPunktMapy(x, y);
      if (x != ptrZ1->z_x[0]
        ||y != ptrZ1->z_y[0])
        printf("Niezgodne wspolrzedne w zwierzu. poz=%u gen=%u\n", a, ileGen);
      if (ptrMapa->pm_zwierz.pm1_poz != a)
        printf("Niezgodne wspolrzedne lub typ na mapie.poz=%u gen=%u\n", a, ileGen);
      }
    }
} // TestMapy

//---------------------------------------------------------------------------
//! Przesu� r�cznie zwierza, wg klawiatury
short WybierzManual(short poz, short x1, short y1, short* destX, short* destY, DEF_ZWIERZ* defZ1)
{
  short typ = WybranyKierunek(); // ustal 1234
  OBIEKTINFO_ZWIERZ* ptrZ1; // zwierz przetwarzany
  PUNKT_MAPY* ptrMapa=NULL;

  ptrZ1 = listaZwierz+poz;
  *destX = x1;
  *destY = y1;
  switch (typ)
    {
    case 1: // g�ra
      x1 +=  0;
      y1 += -1;
      break;

    case 2: // lewo
      x1 += -1;
      y1 +=  0;
      break;

    case 3: // prawo
      x1 += 1;
      y1 += 0;
      break;

    case 4: // d�
      x1 += 0;
      y1 += 1;
      break;
    }
  if (CzyPunktZakres(x1, y1)==0) // punkt spoza mapy
    return 0; // zako�cz, nie mo�na w t� stron�
  if (PunktWlasnyZwierz(ptrZ1, x1, y1))
    return 0; // sam zajmuj� to pole
  if (CzyPunktLadowy(x1, y1)==0) // czy mozna wej�� na ten punkt
    return 0;

  ptrMapa = PtrPunktMapy(x1, y1);

  // sprawd� czy na trasie jest zwierz do zjedzenia
  if (defZ1->dz_drapieznik // jestem drapie�nikiem
    && CzyMapaZwierz(x1, y1) == 1) // jest do zjedzenia ro�lino�erca
    {
    *destX = x1;
    *destY = y1;
    return 2;
    }
  // sprawd� czy jest ro�lina
  if (CzyMapaZwierz(x1, y1) == 0) // czy miejsce jest wolne
    {
    *destX = x1; // na pewno tam id�
    *destY = y1;
    if (ptrMapa->pm_roslina.pm1_tab == TAB_ROSLINA
      &&defZ1->dz_roslinozerca) // mog� to zje��
      return 1;
    }
  return 0; // bez zjadania
} // WybierzManual

//---------------------------------------------------------------------------
// Przetw�rz zwierz� z listy
void ProcessZwierz(short poz, short manual)
{
  short mapSrcX, mapSrcY;
  short mapDestX, mapDestY;
  short zjedz=0;
  short poz2;

  OBIEKTINFO_ZWIERZ* ptrZ1; // zwierz przetwarzany
  DEF_ZWIERZ* defZ1;        // definicja

  OBIEKTINFO_ZWIERZ* ptrZ2; // zwierz zjadany
  DEF_ZWIERZ* defZ2;        // definicja
  DEF_ROSLINA* defR;        // definicja

  ptrZ1 = listaZwierz+poz;
  if (ptrZ1->z_def < 0) // pomijaj martwe (czyli ujemne)
    return;

  defZ1 = defZwierz + (ptrZ1->z_def);
  mapSrcX = ptrZ1->z_common.x;
  mapSrcY = ptrZ1->z_common.y;

  //=== 1.Ustal miejsce do kt�rego ma si� przemie�ci� (oraz czy co� zje)
  if (manual)
    zjedz = WybierzManual(poz, mapSrcX, mapSrcY, &mapDestX, &mapDestY, defZ1); // zwraca 1 lub 2
  else
    zjedz = WybierzFood(mapSrcX, mapSrcY, &mapDestX, &mapDestY, defZ1); // zwraca 1 lub 2

  //=== najpierw zjedz obiekt w punkcie docelowym, potem tam si� przemie��
  if (zjedz)
    {
    if (zjedz == 2) // zjedz ro�lino�erc�
      {
      poz2 = GetPozZwierz(mapDestX, mapDestY);
      ptrZ2 = listaZwierz+poz2;
      defZ2 = defZwierz + ptrZ2->z_def;
      ptrZ1->z_zapas += defZ2->dz_kalorie;
      if (ptrZ1->z_zapas > defZ1->dz_maxZapas)
        ptrZ1->z_zapas = defZ1->dz_maxZapas;
      UsunMartwego(poz2);
      }
    else if (RoslinaJadalna(mapDestX, mapDestY))
      {
      poz2 = GetPozRoslina(mapDestX, mapDestY);
      if (CzyMapaRoslina(mapDestX, mapDestY)) // jest trawa w tabeli
        {
        defR = defRoslina + listaRoslin[poz2].r_def;
        listaRoslin[poz2].r_poziom = 0;
        ptrZ1->z_zapas += defR->dr_kalorie;
        if (ptrZ1->z_zapas > defZ1->dz_maxZapas)
          ptrZ1->z_zapas = defZ1->dz_maxZapas;
        }
      }
    }
  TestMapy();
  if (mapDestX != mapSrcX
    ||mapDestY != mapSrcY) // tylko gdy zmienia miejsce
    {
    if (ptrZ1->z_zapas >= defZ1->dz_maxZapas-1
      &&ptrZ1->z_size  <  defZ1->dz_maxSize)
      {
      ptrZ1->z_zapas -= defZ1->dz_utrata;
      PrzesunZwierz(poz, mapSrcX, mapSrcY, mapDestX, mapDestY, 1); // 1=uro�nij
      }
    else // tylko przesu�
      PrzesunZwierz(poz, mapSrcX, mapSrcY, mapDestX, mapDestY, 0);
    }
  TestMapy();
  // 2. ewentualne rozmna�anie
  if (ptrZ1->z_zapas >= defZ1->dz_maxZapas) // s� nadwy�ki do wydania
    {
    short ok;
    short mapNewX, mapNewY;

    ok = WybierzDlaNowego(mapDestX, mapDestY, &mapNewX, &mapNewY);
    if (ok)
      {
      ptrZ1->z_zapas -= defZ1->dz_utrata; // zmniejsz poziom zapasu matce
      DodajZwierz(mapNewX, mapNewY, ptrZ1->z_def);
      }
    }
  TestMapy();
  //=== 3.na koniec - zmniejsz zapas
  if (ptrZ1->z_zapas > 0)
    ptrZ1->z_zapas--;

  if (ptrZ1->z_zapas <= 0)
    UsunMartwego(poz);
  TestMapy();
} // ProcessZwierz

//---------------------------------------------------------------------------
//! Sprawd� czy wskazany punkt jest ju� zaj�ty przez tego zwierza
short PunktWlasnyZwierz(OBIEKTINFO_ZWIERZ* ptrZ1, short x, short y)
{
  short a;
  for (a=0; a<ptrZ1->z_size; a++)
    if (ptrZ1->z_x[a] == x
      &&ptrZ1->z_y[a] == y)
      return 1; // tak, to w�asny punkt
  return 0; // nie ma mnie tam
} // PunktWlasnyZwierz

//---------------------------------------------------------------------------
//! Czy kierunek wybrany z r�ki? 1234=tak, 0=nie, litery to kody extended x getch()
short WybranyKierunek(void)
{
  switch (kierunek)
    {
    case 'H':
      return 1; // g�ra
    case 'K':
      return 2; // lewo
    case 'M':
      return 3; // prawo
    case 'P':
      return 4; // d�
    }
  return 0; // �aden z nich
} // WybranyKierunek

//---------------------------------------------------------------------------
//! Usu� zw�oki z listy
void UsunMartwe(void)
{
  short x,y, poz;
  PUNKT_MAPY* ptrSrc=NULL;

  for (poz=ileZwierz-1; poz>=0; poz--)
    if (listaZwierz[poz].z_def == -1) // do usuni�cia
      {
      if (poz != (ileZwierz-1)) // to nie jest ostatni
        {
        memmove(listaZwierz+poz, listaZwierz+poz+1, sizeof(OBIEKTINFO_ZWIERZ)*(ileZwierz-poz-1));

        // przenumeruj obiekty na mapie (o -1)
        short a;
        for (a=0; a<ileZwierz-poz-1; a++)
          {
          short seg;
          for (seg=0; seg<listaZwierz[poz+a].z_size; seg++)
            {
            x = listaZwierz[poz+a].z_x[seg];
            y = listaZwierz[poz+a].z_y[seg];
            ptrSrc = PtrPunktMapy(x, y);
            ptrSrc->pm_zwierz.pm1_poz = poz+a; // nowy numer
            //if (ptrSrc->pm_zwierz.pm1_poz < 0)
            //  printf("MINUS");
            }
          }
        }
      ileZwierz--;
      martwe++;
      }
} // UsunMartwe

//---------------------------------------------------------------------------
//! Przetwarzanie obiektow o 1 jednostk� czasu
void PrzetworzMape(void)
{
  short poz, max;

  ileGen++;
  // przegl�danie kolejnych obiekt�w jest szybsze ni� przegl�danie wg mapy
  // nie tracimy czasu na puste kom�rki
  TestMapy();
  max = ileRoslin; // ustal ilo�� przed - nowo dodane nie b�d� uwzgl�dnione
  for (poz=0; poz<max; poz++)
    ProcessRoslina(poz);

  // zwierz�ta
  max = ileZwierz; // ustal ilo�� przed - nowo dodane nie b�d� uwzgl�dnione
  for (poz=0; poz<max; poz++)
    {
    short manual=0;
    if (listaZwierz[poz].z_id == selZwierz) // ten jest przetwarzany
      {
      if (WybranyKierunek() > 0)
        manual = 1;
      }
    ProcessZwierz(poz, manual);
    }
  // usu� martwe z listy
  UsunMartwe();
  // sprawd�, czy bie��cy jeszcze �yje
  if (Id2PozZwierz(selZwierz) < 0)
    selZwierz = -1; // �aden
} // PrzetworzMape

//@} weze_przetwarzanie




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_output
//!  Drukowanie aktualnego stanu mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


// wy�wietlanie zajmuje du�o wi�cej czasu ni� generowananie, wi�c mo�na
// pokazywa� co kt�r�� klatk� - wg zmiennej czestoPokaz
static short czestoPokaz=1; //!< Co kt�r� klatk� pokazywa�?

#define G0 0
#define D0 1
#define L0 2
#define P0 3
//---------------------------------------------------------------------------
//! Ustal po�o�enie punkt�w wzgl�dem siebie
short TypSasiada(short x, short y, short x2, short y2)
{
  if (x == x2) // nad i pod
    {
    if (y > y2)
      return G0;
    else
      return D0;
    }
  if (x > x2)
    return L0;
  else
    return P0;
} // TypSasiada

//---------------------------------------------------------------------------
//! Zamie� sasiedztwo na znak segmentu
char ZnakSegmentu(short* tab)
{
  if (tab[G0] && tab[D0])
    return 186;
  if (tab[L0] && tab[P0])
    return 205;
  if (tab[G0] && tab[P0])
    return 200;
  if (tab[G0] && tab[L0])
    return 188;
  if (tab[D0] && tab[P0])
    return 201;
  if (tab[D0] && tab[L0])
    return 187;

  // dla ogona:
  if (tab[G0] || tab[D0])
    return 186;
  if (tab[L0] || tab[P0])
    return 205;
  return '*';
} // ZnakSegmentu

//---------------------------------------------------------------------------
//! Ustal jaki znak pozwala na pokazanie kszta�tu
char ZnakWeza(short x, short y)
{
  OBIEKTINFO_ZWIERZ* ptrZ;
  char znak = '*';
  short a, poz, t;
  short bs[4];

  poz = GetPozZwierz(x,y);
  ptrZ = listaZwierz+poz;

  // sprawd� kt�ry pasuje, od tego zale�y wygl�d
  for (a=0; a<ptrZ->z_size; a++)
    if (ptrZ->z_x[a] == x
      &&ptrZ->z_y[a] == y) // to ten!
      {
      if (a == 0) // g�owa
        {
        // g�owa ma literk�
        znak = 'A' + ptrZ->z_zapas -1;
        if (ptrZ->z_zapas > 24)
          znak = '$';
        return znak;
        }
      else
        {
        if (debug) // w trybie debug pokazuj cyferki
          return '0'+a;

        // zwyk�e znaki segmentowe
        memset(bs, 0, sizeof(bs));
        t = TypSasiada(ptrZ->z_x[a], ptrZ->z_y[a], ptrZ->z_x[a-1], ptrZ->z_y[a-1]);
        bs[t] = 1;
        if (a < ptrZ->z_size-1) // nie sprawdzaj dla ostatniego
          {
          t = TypSasiada(ptrZ->z_x[a], ptrZ->z_y[a], ptrZ->z_x[a+1], ptrZ->z_y[a+1]);
          bs[t] = 1;
          }
        znak = ZnakSegmentu(bs);
        }
      }
  return znak;
} // ZnakWeza

//---------------------------------------------------------------------------
//! Ustal znak i kolor dla gruntu
char ZnakGruntu(PUNKT_MAPY* ptrMapa, short* kolor)
{
  OBIEKTINFO_GRUNT* ptrG;

  ptrG = listaGrunt + ptrMapa->pm_grunt.pm1_poz;
  switch (ptrG->g_def)
    {
    case G_ROLNA:
      *kolor = FOREGROUND_GREEN;
      return '.';

    case G_OCEAN:
      *kolor = FOREGROUND_GREEN;
      return ' ';

    case G_PUSTKA:
      *kolor = FOREGROUND_GREEN | FOREGROUND_RED;
      return '*';
    }
  *kolor = FOREGROUND_RED;
  return '?';
} // ZnakGruntu

//---------------------------------------------------------------------------
//! Zamie� punkt mapy na literk� do wydruku
void DrukujZnakMapy(short x, short y)
{
  char  znak[2]= "_";
  short poz, kolor;
  PUNKT_MAPY* ptrMapa;
  ptrMapa = PtrPunktMapy(x,y);

  // grunt
  poz = ptrMapa->pm_grunt.pm1_poz;
  znak[0] = ZnakGruntu(ptrMapa, &kolor);
  SetTekstKolor(kolor); //FOREGROUND_RED | FOREGROUND_GREEN);


  // ro�liny
  if (CzyMapaRoslina(x,y)) // jest trawa w tabeli
    {
    poz = GetPozRoslina(x,y);
    znak[0] = '0' + listaRoslin[poz].r_poziom;
    SetTekstKolor(FOREGROUND_RED | FOREGROUND_GREEN /*| FOREGROUND_INTENSITY*/);
    if (RoslinaJadalna(x,y))
      znak[0] = 'o';
    }

  // zwierzeta
  if (CzyMapaZwierz(x,y)) // jest zwierz w tabeli
    {
    poz = ptrMapa->pm_zwierz.pm1_poz;
    if (debug == 0 // pokazuj wszystkie - nie ma trybu debug
      ||listaZwierz[poz].z_id == selZwierz) // bie��cy
      {
      if (listaZwierz[poz].z_def == 0)
        {
        if (listaZwierz[poz].z_id == selZwierz)
          SetTekstKolor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        else
          SetTekstKolor(FOREGROUND_GREEN);
        }
      else // 2
        {
        if (listaZwierz[poz].z_id == selZwierz)
          SetTekstKolor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        else
          SetTekstKolor(FOREGROUND_RED);
        }
      znak[0] = ZnakWeza(x,y);
      }
    }
  PutZnak(znak);
} // DrukujZnakMapy

//---------------------------------------------------------------------------
//! Poka� sk�adniki wskazanego w�a
void PrintWazStats(short poz)
{
  OBIEKTINFO_ZWIERZ* ptrZ;
  short a, ile, reszta;

  ptrZ = listaZwierz+poz;
  printf("\n");
  if (ptrZ->z_zapas > 10)
    SetTekstKolor(FOREGROUND_GREEN);
  else
    SetTekstKolor(FOREGROUND_RED);
  printf("zapas: %2u ", ptrZ->z_zapas);
  ile =ptrZ->z_zapas/10;
  for (a=0; a<ile; a++)
    printf("#");
  ile = ptrZ->z_zapas - (ile*10); // reszta
  for (a=0; a<ile; a++)
    printf(".");
  SetTekstKolor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  printf("                \n");

  printf("z_def: %u  \n", ptrZ->z_def);
  printf("z_size: %u  \n", ptrZ->z_size);
  printf("z_id: %u  \n", ptrZ->z_id);
  printf("x: %u  \n", ptrZ->z_common.x);
  printf("y: %u  \n", ptrZ->z_common.y);
  for (a=0; a<ptrZ->z_size; a++)
    {
    printf("PKT %u: x=%u y=%u           \n", a, ptrZ->z_x[a], ptrZ->z_y[a]);
    }
  printf("====================================");
} // PrintWazStats

//---------------------------------------------------------------------------
//! Wyrzu� stan mapy na standard output, wszystkie zwierza (-1) lub wskazany
void DrukujMape(void)
{
  short x, y;
  short a, wazA, wazB;
  short poz=0;

  if (selZwierz != -1)
    poz = Id2PozZwierz(selZwierz);
  wazA = 0;
  wazB = 0;
  for (a=0; a<ileZwierz;a++)
    {
    OBIEKTINFO_ZWIERZ* ptrZ1 = listaZwierz+a;
    if (ptrZ1->z_def == 0)
      wazA++;
    else
      wazB++;
    }
  SetKursorPoz(0,0);
  printf("MAPA %ux%u: %lu (+%u)      \n", xSize, ySize, ileGen, czestoPokaz);
  printf("weze: %u %u   \n", wazA, wazB);
  printf("dead: %lu   \n", martwe);
  if (poz >= 0)
    printf("wybrany: poz=%u id=%u     ", poz, (unsigned)listaZwierz[poz].z_id);
  else
    printf("nie wybrano                          ");
  printf("\n");
  for (y=0; y<ySize; y++)
    {
    for (x=0; x<xSize; x++)
      DrukujZnakMapy(x,y);
    printf("\n"); // koniec linii
    }
  printf("\n");
  SetTekstKolor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  printf("Wcisnij SPACJE, 1,2,3,4,5 q(koniec)\n");
  printf("d(debug +/-) p(pop) n(nast)\n");
  printf("strzalki - kierunek ruchu\n");
  printf("\n");
  if (selZwierz != -1)
    PrintWazStats(poz);
} // DrukujMape

//---------------------------------------------------------------------------
//! Alokacja pami�ci
void Alokuj(short typ)
{
  if (typ==1)
    {
    mapa = new PUNKT_MAPY[xSize * ySize];
    memset(mapa, 0, sizeof(PUNKT_MAPY)*xSize*ySize);
    listaGrunt  = new OBIEKTINFO_GRUNT[xSize * ySize +1];
    listaRoslin = new OBIEKTINFO_ROSLINA[xSize * ySize +1];
    listaZwierz = new OBIEKTINFO_ZWIERZ[xSize * ySize +1];
    }
  else
    {
    delete[] mapa;
    delete[] listaGrunt;
    delete[] listaRoslin;
    delete[] listaZwierz;
    }
} // Alokuj

/*
Parametry wywo�ania:
1) xSize
2) ySize
3) czestoPokaz
*/
//---------------------------------------------------------------------------
//! G��wne wej�cie do programu
int main(int argc, char* argv[])
{
  short a, znak = ' ';
  char extended;
  short ktory;
  char bufor[5];
  unsigned long ile;
  INPUT_RECORD ir;

  if (argc >= 3) // odczytaj rozmiar z command-line
    {
    a = atoi(argv[1]);
    if (a>3)
      xSize = a;
    a = atoi(argv[2]);
    if (a >3)
      ySize = a;
    }

  if (argc==4) // odczytaj czestotliwo�� z command-line
    {
    a = atoi(argv[3]);
    if (a>1)
      czestoPokaz = a;
    }
  Alokuj(1);
  PrzygotujEkran();
  ileGen = 0;
  PusteTabele();
  ZapelnijMape();
  DrukujMape();
  znak = getch(); // praca krokowa
  _next:
   PrzetworzMape();

  _pokaz: // tylko wy�wietl inaczej, bez przetwarzania
//  system("cls");
  if ((ileGen % czestoPokaz)==0)
    {
    DrukujMape();
    znak = getch(); // praca krokowa
    kierunek = 0;
    if (znak==0)
      {
      kierunek = getch(); // HMKP
      }
    }
  SetTekstKolor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  if (znak == '1')
    czestoPokaz = 1;
  if (znak == '2')
    czestoPokaz = 10;
  if (znak == '3')
    czestoPokaz = 100;
  if (znak == '4')
    czestoPokaz = 1000;
  if (znak == '5')
    czestoPokaz = 10000;
  if (znak == 'd')
    {
    if (debug == 0)
      debug = 1;
    else
      debug = 0;
    }
  if (znak == 'n') // next
    {
    if (selZwierz == -1)
      selZwierz = listaZwierz[0].z_id; // nie by�o �adnego - to id pierwszego
    else
      {
      short poz;
      poz = Id2PozZwierz(selZwierz);
      poz++;
      if (poz >= ileZwierz)
        poz = 0;
      selZwierz = listaZwierz[poz].z_id;
      }
    goto _pokaz;
    }
  if (znak == 'p') // previous
    {
    if (selZwierz == -1)
      selZwierz = listaZwierz[0].z_id; // nie by�o �adnego - to id pierwszego
    else
      {
      short poz;
      poz = Id2PozZwierz(selZwierz);
      poz--;
      if (poz < 0)
        poz = ileZwierz;
      selZwierz = listaZwierz[poz].z_id;
      }
    goto _pokaz;
    }

  if (znak == 'q')
    {
    Alokuj(2);
    return 0;
    }
  goto _next;

// return 0;
} // main

//@} weze_output

// eof: weze.cpp

