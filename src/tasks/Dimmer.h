#ifndef TASK_DIMMER
#define TASK_DIMMER

    #include <Arduino.h>
    #include "../config/config.h"
    #include "../config/enums.h"
    #include "../functions/dimmerFunction.h"


extern HTTPClient http;

extern DisplayValues gDisplayValues;
extern  Config config;
extern dimmerLamp dimmer_hard; 

extern SemaphoreHandle_t xSemaphoreDimmer;

/**
 * Task: Modifier le dimmer en fonction de la production
 * 
 * récupère les informations, conso ou injection et fait varier le dimmer en conséquence
 * 
 */
void get_dimmer_child_power (){
    /// récupération de la puissance du dimmer enfant en http
     
        String baseurl; 
        baseurl = "/state";
        http.begin(String(config.dimmer),80,baseurl);   
        
        int httpResponseCode = http.GET();
        String dimmerstate = "0"; 
        dimmerstate = http.getString();
        http.end();
        
        if (httpResponseCode==200) {
            DynamicJsonDocument doc(128);
            deserializeJson(doc, dimmerstate);
            //int ptotal = doc["Ptotal"];
            
            gDisplayValues.puissance_route = doc["Ptotal"];
  
            Serial.println("Puissance routee    : " + String(gDisplayValues.puissance_route));
        }
        else {
            gDisplayValues.puissance_route = 0;
        }
     

}


void updateDimmer(void * parameter){
  for (;;){
    gDisplayValues.task = true;
    // Wait for semaphore or sleep for 5 seconds, avant de refaire une analyse
    if (xSemaphoreDimmer == NULL) {
        Serial.println("=>SEMAPHORE DIMMER NULL");
        vTaskDelay(pdMS_TO_TICKS(4000));
   }
    else {
        xSemaphoreTake(xSemaphoreDimmer, pdMS_TO_TICKS(4000));
    }

#if WIFI_ACTIVE == true
    #if CLEAN
    /*
    /// si changement à faire
    if  (gDisplayValues.change != 0 ) {
        Serial.println(F("changement des valeurs dimmer-MQTT"));
        // envoie de l'information au dimmer et au serveur MQTT ( mosquito ou autre )
        dimmer_change(); 
    }*/ 
    #endif
   
    // si dimmer local alors calcul de puissance routée 
    if (config.dimmerlocal) {
       gDisplayValues.puissance_route = config.resistance * dimmer_hard.getPower()/100; 
    }
    // si dimmer distant alors calcul de puissance routée
    else  {  
      get_dimmer_child_power ();
    }

    
    dimmer();



#endif
    gDisplayValues.task = false;
    // Sleep for 5 seconds, avant de refaire une analyse
    //vTaskDelay(pdMS_TO_TICKS(4000));
    // 24/01/2023 changement de 5 à 4s 
  }
  vTaskDelete(NULL); //task destructor in case task jumps the stack
}
#endif