// -----------------------------
//  Zlote w�e: struktury danych
// -----------------------------

// w kt�rych tabelach jest przechowywany opis obiektu
#define TAB_GRUNT   1
#define TAB_ROSLINA 2
#define TAB_ZWIERZ  3

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
  short x;  // wsp�rz�dna x na mapie
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
  short dz_kalorie;
  //short defz_maxModul;  // do ilu modu��w mo�e rosn��

  //short defz_zapNowyMod;// ile zasob�w �ywno�ci trzeba na wzrost o 1 modu�
  //short defz_zasieg;    // zasi�g widoczno�ci - do analizy
  } DEF_ZWIERZ;

typedef struct
  {
  short z_def; // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON z_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short z_zapas;  // bie�acy poziom zapasu �ywno�ci
  } OBIEKTINFO_ZWIERZ;


// eof: weze.h

