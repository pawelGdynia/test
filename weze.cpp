//---------------------------
//  weze.cpp - przetwarzanie
//---------------------------

#pragma hdrstop
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

//#include "weze.h"


  // w kt�rych tabelach jest przechowywany opis obiektu
#define TAB_GRUNT   1
#define TAB_ROSLINA 2
#define TAB_ZWIERZ  3

#define MAX_SIZE 15
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
  short dz_maxZapas;  // ile zasobow (trawy) mo�e przechowa�
  short dz_roslinozerca;
  short dz_drapieznik;
  short dz_kalorie;    // ile punktow daje zjedzenie go
  short defz_maxSize;  // do ilu modu��w mo�e rosn��

  //short defz_zapNowyMod;// ile zasob�w �ywno�ci trzeba na wzrost o 1 modu�
  //short defz_zasieg;    // zasi�g widoczno�ci - do analizy
  } DEF_ZWIERZ;

typedef struct
  {
  short z_def; // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON z_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short z_zapas;  // bie�acy poziom zapasu �ywno�ci
  short z_x[MAX_SIZE+1];// wsp�rz�dne wszystkich segment�w
  short z_y[MAX_SIZE+1];
  short z_size;
  } OBIEKTINFO_ZWIERZ;


//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_przetwarzanie
//!  Alogorytmy przetwarzania obiekt�w i mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static short xSize = 60;
static short ySize = 18;

static PUNKT_MAPY* mapa;
static DEF_GRUNT defGrunt[] = {
  {0,0}, // 0, ziemia rolna
  {1,1}, // 1, ocean
  {0,1}  // 2, pustynia
  };
#define ILE_DEFGRUNT (sizeof(defGrunt)/sizeof(defGrunt[0]))

static DEF_ROSLINA defRoslina[] = {
  {9, 2},
  };
#define ILE_DEFROSLINA (sizeof(defRoslina)/sizeof(defRoslina[0]))

static DEF_ZWIERZ defZwierz[] = {
  {10, 1, 0, 3, 10}, // 0: ro�lino�erca
  {10, 1, 1, 3, 10}, // 1: drapie�nik
  };
#define ILE_DEFZWIERZ (sizeof(defZwierz)/sizeof(defZwierz[0]))

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
  memset(listaGrunt,  0, sizeof(listaGrunt));
  memset(listaRoslin, 0, sizeof(listaRoslin));
  memset(listaZwierz, 0, sizeof(listaZwierz));
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
  PUNKT_MAPY* ptr;
  if (ileRoslin >= xSize * ySize)
    {
    printf("E:DodajRosline()");
    return; // nie dodawaj - nie ma miejsca
    }
  ptr = PtrPunktMapy(x,y);
  // info w samej mapie
  ptr->pm_roslina.pm1_tab = TAB_ROSLINA;
  ptr->pm_roslina.pm1_poz = ileRoslin;

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

  if (ileZwierz >= xSize * ySize)
    {
    printf("E:DodajZwierz()");
    return; // nie dodawaj - nie ma miejsca
    }
  ptrMapa = PtrPunktMapy(x,y);
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
  ileZwierz++;
} // DodajZwierz

//---------------------------------------------------------------------------
//! Wype�nij struktury pierwsz� generacj� obiekt�w
void ZapelnijMape(void)
{
  short x,y;

  for (x=0; x<xSize; x++)
    for (y=0; y<ySize; y++)
      {
      // grunty rolne - wsz�dzie uprawne (typ 0)
      DodajGrunt(x, y, 0);

      // ro�liny
      //if (y%2) // tylko kilka rz�dk�w ro�liny zwyk�ej (typ 0)
        DodajRosline(x, y, 0);
      if (x==y)
        DodajZwierz(x, y, 0); // gatunek "0"
      }
// DodajZwierz(1, 1, 0);
 DodajZwierz(7, 8, 1);
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
//  a = 11;
  num4[0] = losowe[a][0];
  num4[1] = losowe[a][1];
  num4[2] = losowe[a][2];
  num4[3] = losowe[a][3];
} // UstalRandom4

//---------------------------------------------------------------------------
//! Sprawd� czy zadane wsp�rz�dne mieszcz� si� na mapie
short CzyPunktZakres(short x, short y)
{
  if (x < 0
    ||y < 0
    ||x >= xSize
    ||y >= ySize)
    return 0; // poza zakresem - tam nic nie ma!
  return 1; // ok
} // CzyPunktZakres

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wsp�rz�dnych wyst�puje obiekt - ro�lina
short CzyMapaRoslina(short x, short y)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  if (ptr->pm_roslina.pm1_tab == 0)
    return 0; // nie ma tam ro�liny

  return 1; // jest
} // CzyMapaRoslina

//---------------------------------------------------------------------------
//! Kt�r� pozycj� w tabeli zajmuje ro�lina?
short GetPozRoslina(short x, short y)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  return ptr->pm_roslina.pm1_poz;
} // GetPozRoslina

//---------------------------------------------------------------------------
//! Sprawdza czy dla podanych wsp�rz�dnych zwierz zwyk�y (1) czy drapie�nik (2)
short CzyMapaZwierz(short x, short y)
{
  PUNKT_MAPY* ptr;
  DEF_ZWIERZ* ptrDef;      // definicja
  ptr = PtrPunktMapy(x,y);

  if (ptr->pm_zwierz.pm1_tab == 0)
    return 0; // nie ma tam nikogo
  ptrDef = defZwierz + listaZwierz[ptr->pm_zwierz.pm1_poz].z_def;
  if (ptrDef->dz_drapieznik > 0)
    return 2; // jest drapie�nik

  return 1; // jest ro�lino�erca
} // CzyMapaZwierz

//---------------------------------------------------------------------------
//! Kt�r� pozycj� w tabeli zajmuje zwierz?
short GetPozZwierz(short x, short y)
{
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  return ptr->pm_zwierz.pm1_poz;
} // GetPozZwierz

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
  ptrDef    = defRoslina + ptrRoslina->r_def;

  //-----------
  if (ptrRoslina->r_poziom == ptrDef->dr_czasWzrostu)
    return 1; // tylko maksymalne s� jadalne

  return 0;
} // RoslinaJadalna

//---------------------------------------------------------------------------
//! Wybierz z s�siedztwa miejsce do zjedzenia, wynik=1 oznacza �e wybrano
short WybierzFood(short x1, short y1, short* destX, short* destY, DEF_ZWIERZ* def)
{
  short a;
  short kolej[4];
//  short lastX=x1;
//  short lastY=y1;

  UstalRandom4(kolej);
  // w losowej kolejno�ci - sprawd� punkty s�siednie
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
    if (CzyPunktZakres(*destX, *destY)) // punkt nie wyszed� poza map�
      {
      if (def->dz_drapieznik)
        {
        if (CzyMapaZwierz(*destX, *destY)==1) // znalaz� ro�lino�erc�
          return 2; // zjedz go!
        }
      else // drapie�nik nie jada ro�lin
        {
        if (def->dz_roslinozerca
          &&CzyMapaZwierz(*destX, *destY)==0)
          {
          //lastX = *destX;
          //lastY = *destY;
          if (RoslinaJadalna(*destX, *destY))
            return 1; // wybierz ten punkt
          }
        }
      }
    }
  // Sprawd� czy w punkcie bie��cym jest ro�lina
  if (def->dz_roslinozerca
    &&RoslinaJadalna(x1, y1))
    {
    *destX = x1;
    *destY = y1;
    return 1;
    }

  // przyjmij inny punkt w otoczeniu - wolny
//  *destX = lastX;
//  *destY = lastY;
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
//  ptrSrc = PtrPunktMapy(srcX, srcY);
//  ptrMapa= PtrPunktMapy(dstX, dstY);

  //====== MAPA: przesuwanie info o g�owie =====================
  // przepisz dane ze starego miejsca do nowego
//  ptrDst->pm_zwierz.pm1_tab = ptrSrc->pm_zwierz.pm1_tab;
//  ptrDst->pm_zwierz.pm1_poz = ptrSrc->pm_zwierz.pm1_poz;

  // wyma� w starym miejscu na mapie
//  ptrSrc->pm_zwierz.pm1_tab = 0;
//  ptrSrc->pm_zwierz.pm1_poz = 0;

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
// Przetw�rz zwierz� z listy
void ProcessZwierz(short poz)
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

  //=== Ustal miejsce do kt�rego ma si� przemie�ci� (oraz czy co� zje)
  zjedz  = WybierzFood(mapSrcX, mapSrcY, &mapDestX, &mapDestY, defZ1); // zwraca 1 lub 2

  //=== najpierw zjedz obiekt w punkcie docelowym, potem tam si� przemie��
  if (zjedz)
    {
    if (zjedz==2) // zjedz ro�lino�erc�
      {
      poz2 = GetPozZwierz(mapDestX, mapDestY);
      ptrZ2 = listaZwierz+poz2;
      if (ptrZ1->z_zapas < defZ1->dz_maxZapas)
        {
        // tylko gdy nie jest pe�ny
        defZ2 = defZwierz + ptrZ2->z_def;
        ptrZ1->z_zapas += defZ2->dz_kalorie;
        ptrZ2->z_zapas = 0; // wyzeruj zjedzonego
        ptrZ2->z_def= -1;
        }
      }
    else if (RoslinaJadalna(mapDestX, mapDestY))
      {
      poz2 = GetPozRoslina(mapDestX, mapDestY);
      if (CzyMapaRoslina(mapDestX, mapDestY)) // jest trawa w tabeli
        {
        defR = defRoslina + listaRoslin[poz2].r_def;
        listaRoslin[poz2].r_poziom = 0;
        if (ptrZ1->z_zapas < defZ1->dz_maxZapas)
          {
          // tylko gdy nie jest pe�ny
          ptrZ1->z_zapas += defR->dr_kalorie;
          }
        }
      }
    }

  if (mapDestX != mapSrcX
    ||mapDestY != mapSrcY) // tylko gdy zmienia miejsce
    {
    if (listaZwierz[poz].z_zapas > 8
      &&listaZwierz[poz].z_size < MAX_SIZE)
      {
      listaZwierz[poz].z_zapas -= 5;
      PrzesunZwierz(poz, mapSrcX, mapSrcY, mapDestX, mapDestY, 1); // 1=uro�nij
      }
    else // tylko przesu�
      PrzesunZwierz(poz, mapSrcX, mapSrcY, mapDestX, mapDestY, 0);
    }

  //=== zmniejsz zapas
  if (listaZwierz[poz].z_zapas > 0)
    listaZwierz[poz].z_zapas--;

  if (listaZwierz[poz].z_zapas <= 0)
    UsunMartwego(poz);

  // rozmna�anie
  if (ptrZ1->z_zapas >= defZ1->dz_maxZapas) // s� nadwy�ki do wydania
    {
    short ok;
    short mapNewX, mapNewY;

    ok = WybierzDlaNowego(mapDestX, mapDestY, &mapNewX, &mapNewY);
    if (ok)
      {
      ptrZ1->z_zapas -= 5; // zmniejsz poziom zapasu matce
      DodajZwierz(mapNewX, mapNewY, ptrZ1->z_def);
      }
    }
} // ProcessZwierz


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
        memmove(listaZwierz+poz, listaZwierz+poz+1, sizeof(listaZwierz[0])*(ileZwierz-poz-1));

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
            ptrSrc->pm_zwierz.pm1_poz--; // przenumeruj o 1 w d�
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

  max = ileRoslin; // ustal ilo�� przed - nowo dodane nie b�d� uwzgl�dnione
  for (poz=0; poz<max; poz++)
    ProcessRoslina(poz);

  // zwierz�ta
  max = ileZwierz; // ustal ilo�� przed - nowo dodane nie b�d� uwzgl�dnione
  for (poz=0; poz<max; poz++)
    ProcessZwierz(poz);

  // usu� martwe z listy
  UsunMartwe();

} // PrzetworzMape

//@} weze_przetwarzanie




//   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//!  @name weze_output
//!  Drukowanie aktualnego stanu mapy
//@{ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static HANDLE hStdOut;

// wy�wietlanie zajmuje du�o wi�cej czasu ni� generowananie, wi�c mo�na
// pokazywa� co kt�r�� klatk� - wg zmiennej czestoPokaz
static short czestoPokaz=1; //!< Co kt�r� klatk� pokazywa�?
//---------------------------------------------------------------------------
//! Ustaw kolor tekstu - kolor zdefiniowany w windows.h
void SetTextColor(short kolor)
{
  SetConsoleTextAttribute(hStdOut, kolor);
} // SetTextColor

//---------------------------------------------------------------------------
//! Drukuj tekst wg bie��cego atrybutu
void PutZnak(char* znak)
{
  unsigned long count;
  WriteConsole(hStdOut, znak, 1, &count, NULL);
} // PutZnak

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
        //znak = 'A' + ptrZ->z_zapas -1;
        znak = '8';
        return znak;
        }
/*      if (a == ptrZ->z_size-1)
        {
        // koniec ogona to kropka
        znak = '.';
        return znak;
        }*/
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
  return znak;
} // ZnakWeza

//---------------------------------------------------------------------------
//! Zamie� punkt mapy na literk� do wydruku
void DrukujZnakMapy(short x, short y)
{
  char  znak[2]= "_";
  short poz;
  PUNKT_MAPY* ptr;
  ptr = PtrPunktMapy(x,y);

  // grunt
  poz = ptr->pm_grunt.pm1_poz;
  if (ptr->pm_grunt.pm1_tab != 0)
    {
    SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN);
    strcpy(znak, ".");
    }

  // ro�liny
  if (CzyMapaRoslina(x,y)) // jest trawa w tabeli
    {
    poz = GetPozRoslina(x,y);
    znak[0] = '0' + listaRoslin[poz].r_poziom;
    if (RoslinaJadalna(x,y))
      {
      SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN /*| FOREGROUND_INTENSITY*/);
      znak[0] = 'o';
      }
    else
      SetTextColor(FOREGROUND_GREEN);
    }

  // zwierzeta
  if (CzyMapaZwierz(x,y)) // jest zwierz w tabeli
    {
    poz = ptr->pm_zwierz.pm1_poz;    
    znak[0] = ZnakWeza(x,y);
    if (listaZwierz[poz].z_def == 0)
      SetTextColor(FOREGROUND_BLUE);
    else // 2
      SetTextColor(FOREGROUND_RED);
    }
  PutZnak(znak);
} // DrukujZnakMapy

//---------------------------------------------------------------------------
//! Wyrzu� stan mapy na standard output
void DrukujMape(void)
{
  short x, y;
  short a, wazA, wazB;
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
  COORD coord = {0,0};
  SetConsoleCursorPosition(hStdOut, coord);
  printf("MAPA %ux%u: %lu (+%u)      \n", xSize, ySize, ileGen, czestoPokaz);
  printf("weze: %u %u   \n", wazA, wazB);
  printf("dead: %lu   \n", martwe);
  for (y=0; y<ySize; y++)
    {
    for (x=0; x<xSize; x++)
      DrukujZnakMapy(x,y);
    printf("\n"); // koniec linii
    }
  printf("\n");
  SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  printf("Wcisnij SPACJE, 1,2,3,4,5 q(koniec)");
  printf("\n");  
} // DrukujMape

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
  if (argc >= 3)
    {
    a = atoi(argv[1]);
    if (a>3)
      xSize = a;
    a = atoi(argv[2]);
    if (a >3)
      ySize = a;
    }
  if (argc==4)
    {
    a = atoi(argv[3]);
    if (a>1)
      czestoPokaz = a;
    }
  mapa = new PUNKT_MAPY[xSize * ySize];
  memset(mapa, 0, sizeof(PUNKT_MAPY)*xSize*ySize);
  listaGrunt  = new OBIEKTINFO_GRUNT[xSize * ySize +1];
  listaRoslin = new OBIEKTINFO_ROSLINA[xSize * ySize +1];
  listaZwierz = new OBIEKTINFO_ZWIERZ[xSize * ySize +1];

  clrscr();
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  ileGen = 0;
  PusteTabele();
  ZapelnijMape();
  DrukujMape();
  znak = getch(); // praca krokowa

  _next:
  PrzetworzMape();
  if ((ileGen % czestoPokaz)==0)
    {
    DrukujMape();
    znak = getch(); // praca krokowa
    }
  SetTextColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
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

  if (znak == 'q')
    {
    delete[] mapa;
    delete[] listaGrunt;
    delete[] listaRoslin;
    delete[] listaZwierz;
    return 0;
    }
  goto _next;

// return 0;
} // main

//@} weze_output

// eof: weze.cpp

