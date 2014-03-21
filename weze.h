// -----------------------------
//  Zlote wê¿e: struktury danych
// -----------------------------

// w których tabelach jest przechowywany opis obiektu
#define TAB_GRUNT   1
#define TAB_ROSLINA 2
#define TAB_ZWIERZ  3

typedef struct
  {
  short pm1_tab; // TAB_*: w której tabeli obiektów znajduje siê opis
  short pm1_poz; // pozycja w tabeli dla wskazanego typu 
  } PUNKT_MAPY1;

// dane 1 punktu na mapie
typedef struct
  {
  PUNKT_MAPY1 pm_grunt;
  PUNKT_MAPY1 pm_roslina;
  PUNKT_MAPY1 pm_zwierz;
  } PUNKT_MAPY;

// W³aœciwoœci WSPÓLNE
typedef struct
  {
  short x;  // wspó³rzêdna x na mapie
  short y;  // wspó³rzêdna y na mapie
  short po; // czy po przetworzeniu
  } OBIEKTINFO_COMMON;

//=== GRUNTY ===============================================================
typedef struct // info wspólne dla wszystkich wyst¹pieñ
  {
  short dg_blok; // miejsce niedostêpne - ani roœlin ani zwierz¹t
  short dg_dead; // miejsce martwe - bez roœlin, ale dostêpne dla zwierz¹t
  } DEF_GRUNT;

typedef struct // info dla jednego punktu
  {
  short g_def;              // która definicja w³aœciwoœci
  OBIEKTINFO_COMMON g_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu: brak
  } OBIEKTINFO_GRUNT;

//== ROŒLINY ===============================================================
typedef struct // info wspólne dla wszystkich wyst¹pieñ
  {
  short dr_czasWzrostu; // max poziom, ile okresów na pe³ny wzrost od zera
  short dr_kalorie;     // ile punktów zyskuje zjadaj¹cy
  } DEF_ROSLINA;

typedef struct
  {
  short r_def; // która definicja w³aœciwoœci
  OBIEKTINFO_COMMON r_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short r_poziom; // bie¿acy poziom wzrostu - max to defr_czasWzrostu
  } OBIEKTINFO_ROSLINA;

//=== ZWIERZÊTA ============================================================
typedef struct // info wspólne dla wszystkich wyst¹pieñ
  {
  short dz_maxZapas;  // ile zasobow (trawy) mo¿e przechowaæ
  short dz_roslinozerca;
  short dz_drapieznik;
  short dz_kalorie;
  //short defz_maxModul;  // do ilu modu³ów mo¿e rosn¹æ

  //short defz_zapNowyMod;// ile zasobów ¿ywnoœci trzeba na wzrost o 1 modu³
  //short defz_zasieg;    // zasiêg widocznoœci - do analizy
  } DEF_ZWIERZ;

typedef struct
  {
  short z_def; // która definicja w³aœciwoœci
  OBIEKTINFO_COMMON z_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short z_zapas;  // bie¿acy poziom zapasu ¿ywnoœci
  } OBIEKTINFO_ZWIERZ;


// eof: weze.h

