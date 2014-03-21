// -----------------------------
//  Zlote wê¿e: struktury danych
// -----------------------------

#define TYPOB_GRUNT   1
#define TYPOB_ROSLINA 2
#define TYPOB_ZWIERZ  3

typedef struct
  {
  short pm1_typ; // TYPOB_*: w której tabeli obiektów znajduje siê opis
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

// W³aœciwoœci WSPÓLNE
typedef struct
  {
  short oic_x;  // wspó³rzêdna x
  short oic_y;  // wspó³rzêdna y
  short oic_po; // czy po przetworzeniu
  } OBIEKTINFO_COMMON;

//=== GRUNTY
typedef struct // info wspólne dla wszystkich wyst¹pieñ
  {
  short defg_id;   // identyfikator

  short defg_blok; // miejsce niedostêpne - ani roœlin ani zwierz¹t
  short defg_dead; // miejsce martwe - bez roœlin, ale dostêpne dla zwierz¹t
  } DEF_GRUNT;

typedef struct // info dla jednego punktu
  {
  short oig_defid;              // która definicja w³aœciwoœci
  OBIEKTINFO_COMMON oig_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu: brak
  } OBIEKTINFO_GRUNT;

//== ROŒLINY
typedef struct // info wspólne dla wszystkich wyst¹pieñ
  {
  short defr_id;   // identyfikator

  short defr_czasWzrostu; // max poziom, ile okresów na pe³ny wzrost od zera
  } DEF_ROSLINA;

typedef struct
  {
  short oir_defid; // która definicja w³aœciwoœci
  OBIEKTINFO_COMMON oir_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short oir_poziom; // bie¿acy poziom wzrostu - max to defr_czasWzrostu
  } OBIEKTINFO_ROSLINA;

//=== ZWIERZÊTA
typedef struct // info wspólne dla wszystkich wyst¹pieñ
  {
  short defz_id;   // identyfikator

  short defz_maxModul;  // do ilu modu³ów mo¿e rosn¹æ
  short defz_maxZapas;  // ile zasobow (trawy) mo¿e przechowaæ
  short defz_zapNowyMod;// ile zasobów ¿ywnoœci trzeba na wzrost o 1 modu³
  short defz_zasieg;    // zasiêg widocznoœci - do analizy
  } DEF_ZWIERZ;

typedef struct
  {
  short oiz_defid; // która definicja w³aœciwoœci
  OBIEKTINFO_COMMON oiz_common; // informacje bazowe, systemowe

  // dane indywidualne obiektu
  short oiz_zapas; // bie¿acy poziom zapasu ¿ywnoœci
  } OBIEKTINFO_ZWIERZ;


// eof: weze.h

