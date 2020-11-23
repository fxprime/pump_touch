/* -------------------------------------------------------------------------- */
/*                           State machine variables                          */
/* -------------------------------------------------------------------------- */

#define FORWARD_1 1
#define BACKWARD_1 2
#define FORWARD_2 3
#define FORWARD_3 4


void startup_in();
void identify_in(); 
void home_in();
void setting_in();
void report_in();
void userst_in();
void userko_in();
void oilmet_in();
void print_in();
void caladj_in();
void clkset_in();

void startup_l();
void identify_l(); 
void home_l();
void setting_l();
void report_l();
void userst_l();
void userko_l();
void oilmet_l();
void print_l();
void caladj_l();
void clkset_l();

void init_out();
void identify_out();


/* -------------------------------------------------------------------------- */
/*                              State define here                             */
/* -------------------------------------------------------------------------- */

State st_init         (NULL,        NULL,      init_out);
State st_startup      (startup_in,  startup_l, NULL);
State st_identify     (identify_in, identify_l, identify_out);
State st_home         (home_in,     home_l, NULL);
State st_setting      (setting_in,  setting_l, NULL);
State st_checkreport  (report_in,   report_l, NULL); 
State st_userst       (userst_in,   userst_l, NULL);
State st_userko       (userko_in,   userko_l, NULL);
State st_oilmet       (oilmet_in,   oilmet_l, NULL);
State st_print        (print_in,    print_l, NULL);
State st_caladj       (caladj_in,   caladj_l, NULL);
State st_clkset       (clkset_in,   clkset_l, NULL);
Fsm fsm(&st_init);





/* -------------------------------------------------------------------------- */
/*                        Transition callback functions                       */
/* -------------------------------------------------------------------------- */
#include "state_folder/st_init.h"
#include "state_folder/st_startup.h"
#include "state_folder/st_identify.h"
#include "state_folder/st_home.h"
#include "state_folder/st_setting.h"
#include "state_folder/st_checkreport.h"
#include "state_folder/st_userst.h"
#include "state_folder/st_userko.h"
#include "state_folder/st_oilmet.h"
#include "state_folder/st_print.h"
#include "state_folder/st_caladj.h"
#include "state_folder/st_clkset.h"


 
 


/* -------------------------------------------------------------------------- */
/*                            Transition definition                           */
/* -------------------------------------------------------------------------- */

void states_init() {
  fsm.add_timed_transition(&st_init,  &st_startup,      3000,       NULL);
  fsm.add_transition(&st_startup,     &st_identify,     FORWARD_1,  NULL);

  fsm.add_transition(&st_identify,    &st_home,         FORWARD_1,  NULL); 

  fsm.add_transition(&st_home,        &st_setting,      FORWARD_1,  NULL);
  fsm.add_transition(&st_home,        &st_checkreport,  FORWARD_2,  NULL); 
  fsm.add_transition(&st_home,        &st_userst,       FORWARD_3,  NULL); 
  fsm.add_transition(&st_home,        &st_identify,     BACKWARD_1,   NULL);

  fsm.add_transition(&st_setting,     &st_home,         BACKWARD_1,   NULL);
  fsm.add_transition(&st_setting,     &st_clkset,         FORWARD_1,   NULL);

  fsm.add_transition(&st_checkreport, &st_home,         BACKWARD_1,   NULL); 

  fsm.add_transition(&st_userst,      &st_home,         BACKWARD_1,  NULL); 
  fsm.add_transition(&st_userst,      &st_userko,       FORWARD_1,  NULL); 
  
  fsm.add_transition(&st_userko,      &st_oilmet,       FORWARD_1,  NULL); 
  fsm.add_transition(&st_userko,      &st_userst,       BACKWARD_1,  NULL); 
  
  fsm.add_transition(&st_oilmet,      &st_print,      FORWARD_1,  NULL); 
  fsm.add_transition(&st_oilmet,      &st_userko,       BACKWARD_1,  NULL); 
  fsm.add_transition(&st_oilmet,      &st_caladj,       FORWARD_2,  NULL);
  
  fsm.add_transition(&st_print,      &st_userko,      BACKWARD_1,  NULL); 

  fsm.add_transition(&st_caladj,      &st_home,         FORWARD_1,  NULL);
  
  fsm.add_transition(&st_clkset,      &st_home,         BACKWARD_1,  NULL);



  
 
}