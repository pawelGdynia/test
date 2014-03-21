// -----------------------------
//  Zlote w�e: struktury danych
// -----------------------------

#define TYPOB_GRUNT   1
#define TYPOB_ROSLINA 2
#define TYPOB_ZWIERZ  3

typedef struct
  {
  short pm1_typ; // TYPOB_*: w kt�rej tabeli obiekt�w znajduje si� opis
  short pm1_poz; // pozycja w tabeli dla wskazanego typu 
  } PUNKT_MAPY1;

// dane 1 punktu na mapie
typedef struct
  {
  PUNKT_MAPY1 pm_grunt;
  PUNKT_MAPY1 pm_roslina;
  PUNKT_MAPY1 pm_zwierz;
  } PUNKT_MAPY;

#define X_SIZE 70
#define Y_SIZE 50

// W�a�ciwo�ci WSP�LNE
typedef struct
  {
  short oic_x;  // wsp�rz�dna x
  short oic_y;  // wsp�rz�dna y
  short oic_po; // czy po przetworzeniu
  } OBIEKTINFO_COMMON;

//=== GRUNTY
typedef struct // info wsp�lne dla wszystkich wyst�pie�
  {
  short defg_id;   // identyfikator

  short defg_blok; // miejsce niedost�pne - ani ro�lin ani zwierz�t
  short defg_dead; // miejsce martwe - bez ro�lin, ale dost�pne dla zwierz�t
  } DEF_GRUNT;

typedef struct // info dla jednego punktu
  {
  short oig_defid;              // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON oig_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu: brak
  } OBIEKTINFO_GRUNT;

//== RO�LINY
typedef struct // info wsp�lne dla wszystkich wyst�pie�
  {
  short defr_id;   // identyfikator

  short defr_czasWzrostu; // max poziom, ile okres�w na pe�ny wzrost od zera
  } DEF_ROSLINA;

typedef struct
  {
  short oir_defid; // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON oir_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short oir_poziom; // bie�acy poziom wzrostu - max to defr_czasWzrostu
  } OBIEKTINFO_ROSLINA;

//=== ZWIERZ�TA
typedef struct // info wsp�lne dla wszystkich wyst�pie�
  {
  short defz_id;   // identyfikator

  short defz_maxModul;  // do ilu modu��w mo�e rosn��
  short defz_maxZapas;  // ile zasobow (trawy) mo�e przechowa�
  short defz_zapNowyMod;// ile zasob�w �ywno�ci trzeba na wzrost o 1 modu�
  short defz_zasieg;    // zasi�g widoczno�ci - do analizy
  } DEF_ZWIERZ;

typedef struct
  {
  short oiz_defid; // kt�ra definicja w�a�ciwo�ci
  OBIEKTINFO_COMMON oiz_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short oiz_zapas; // bie�acy poziom zapasu �ywno�ci
  } OBIEKTINFO_ZWIERZ;


// eof: weze.h

