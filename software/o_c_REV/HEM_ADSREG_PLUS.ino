// Copyright (c) 2018, Jason Justian
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


/*
  ghostils:
  Envelopes are now independent for control and mod source/destination allowing two individual ADSR's with Release MOD CV input per hemisphere.  
  * CV mod is now limited to release for each channel
  * Output Level indicators have been shrunk to make room for additional on screen indicators for which envelope you are editing.
  * Switching between envelopes is currently handled by simply pressing the encoder button until you pass the release stage on each envelope which will toggle the active envelope you are editing
  * Envelope is indicated by A or B just above the ADSR segments.
  * 
  * TODO Features/UI: 
  * Implement Menu System to select which EG and parameters to edit. (Attempt to keep this as one screen if possible). Would like to keep visual envelop editing, may need to implement timer based menu return with interruption time out.
  * 
  * Add CV Routing selection to any STAGE.
  * Add Envelope output scaling to not require external attenuator/attenuverter for flexibility. 
  * Add Gate triggered modulation source that can be assigned to any specific stage of the envelope and controlled by delayed number of gates or simple random probability.
  * Underpants
  * ???
  * Profit?
  
  
*/

#define HEM_EG_ATTACK 0
#define HEM_EG_DECAY 1
#define HEM_EG_SUSTAIN 2
#define HEM_EG_RELEASE 3
#define HEM_EG_NO_STAGE -1
#define HEM_EG_MAX_VALUE 255

#define HEM_SUSTAIN_CONST 35
#define HEM_EG_DISPLAY_HEIGHT 30

//-ghostils: DEFINE Main menu inactivity timeout ~5secs this will return the user to the main menu:
#define HEM_EG_UI_MENU_TIMEOUT_TICKS 41666

// About four seconds
#define HEM_EG_MAX_TICKS_AD 33333

// About eight seconds
#define HEM_EG_MAX_TICKS_R 133333


//-ghostils: Create menu items enum and some defines:
#define HEM_EG_UI_FIRST_MENU_ITEM 0 
#define HEM_EG_UI_LAST_MENU_ITEM  9
#define HEM_EG_UI_FIRST_CV_MOD_DEST_ITEM 0
#define HEM_EG_UI_LAST_CV_MOD_DEST_ITEM 3

#define HEM_EG_UI_MAIN_MENU 0
#define HEM_EG_UI_EG1_MENU 1
#define HEM_EG_UI_EG2_MENU 2
#define HEM_EG_UI_CV1_DEST_MENU 3
#define HEM_EG_UI_CV2_DEST_MENU 4
#define HEM_EG_UI_EG1_ENV_OUT_STR 5
#define HEM_EG_UI_EG2_ENV_OUT_STR 6
#define HEM_EG_UI_SCREEN_WIDTH 64
#define HEM_EG_UI_SCREEN_HEIGHT 64
#define HEM_EG_UI_CHAR_WIDTH 8
#define HEM_EG_UI_CHAR_HEIGHT 8
#define HEM_EG_UI_MENU_ITEM_WIDTH 24
#define HEM_EG_UI_ENCODER_RIGHT 1
#define HEM_EG_UI_ENCODER_LEFT -1

typedef enum{
    EG1_Edit = 0,
    EG1_CV1_Mod_Dest = 1,    
    EG1_GateTriggered_Mod_Dest = 2,
    EG1_GateTriggered_Mod_Type = 3,
    EG1_Env_Out_Strength = 4,
    EG2_Edit = 5,
    EG2_CV2_Mod_Dest = 6,    
    EG2_GateTriggered_Mod_Dest = 7,
    EG2_GateTriggered_Mod_Type = 8,
    EG2_Env_Out_Strength = 9
}menuItems_t;

//-ghostils:Modulation Selections:
typedef enum{
  ATTACK = 0,
  DECAY = 1,
  SUSTAIN = 2,
  RELEASE = 3
}cvModDest_t;

class ADSREGPLUS : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "ADSR PLUS";
    }

    void Start() {
        edit_stage = 0;        
        //attack = 20;
        //decay = 30;
        //sustain = 120;
        //release = 25;
        ForEachChannel(ch)
        {
            stage_ticks[ch] = 0;
            gated[ch] = 0;
            stage[ch] = HEM_EG_NO_STAGE;
            
            //-ghostils:Initialize ADSR and other 2 value arrays independently
            attack[ch] = 20;
            decay[ch] = 30;
            sustain[ch] = 120;
            release[ch] = 25;          
            release_mod[ch] = 0;

            //-ghostils:Default Envelope Output to 100% on each channel, allow user to scale as needed:
            curCVAMPStr[ch] = 100;
        }

        //-ghostils:Multiple ADSR Envelope Tracking:
        curEG = 0;

        //-ghostils: Init UI Items: 
        curMenu = 0;
        curMenuItem = 0;
        curMenuTimeOutTicks = 0;
        curCV1ModDest = 0;
        curCV1ModDestItem = 0;
        curCV2ModDest = 0;
        curCV2ModDestItem = 0;
        
        //-ghostils: Init mod destination values: 
        for(int i = 0; i < 2; i++){
          for(int j = 0; j < 4; j++){
              cvModDestVal[i][j] = 0;                                 
          }
        }

        cv1 = 0;
        cv2 = 0;

        
    }

    void Controller() {            
        // Look for CV modification        
        //attack_mod = get_modification_with_input(0);        
        //release_mod[0] = get_modification_with_input(1);

        //-ghostils: Update CV1/CV2 to support release only but on each ADSR independently:
        //release_mod[0] = get_modification_with_input(0);
        //release_mod[1] = get_modification_with_input(1);
        cv1 = get_modification_with_input(0);
        cv2 = get_modification_with_input(1);

        //-ghostils: CV Destitinations:
        cvModDestVal[0][curCV1ModDestItem] = cv1;
        cvModDestVal[1][curCV2ModDestItem] = cv2;

        ForEachChannel(ch)
        {
            if (Gate(ch)) {
                if (!gated[ch]) { // The gate wasn't on last time, so this is a newly-gated EG
                    stage_ticks[ch] = 0;
                    if (stage[ch] != HEM_EG_RELEASE) amplitude[ch] = 0;
                    stage[ch] = HEM_EG_ATTACK;
                    AttackAmplitude(ch);
                } else { // The gate is STILL on, so process the appopriate stage
                    stage_ticks[ch]++;
                    if (stage[ch] == HEM_EG_ATTACK) AttackAmplitude(ch);
                    if (stage[ch] == HEM_EG_DECAY) DecayAmplitude(ch);
                    if (stage[ch] == HEM_EG_SUSTAIN) SustainAmplitude(ch);
                }
                gated[ch] = 1;
            } else {
                if (gated[ch]) { // The gate was on last time, so this is a newly-released EG
                    stage[ch] = HEM_EG_RELEASE;
                    stage_ticks[ch] = 0;
                }

                if (stage[ch] == HEM_EG_RELEASE) { // Process the release stage, if necessary
                    stage_ticks[ch]++;
                    ReleaseAmplitude(ch);
                }
                gated[ch] = 0;
            }
                      
            //-ghostils:Attenuate/Proportion Output based on EG strength value: Lock to 0 - MAX CV:
            //Out(ch, GetAmplitudeOf(ch));                        
            int signal = Proportion(GetAmplitudeOf(ch),100,curCVAMPStr[ch]);                                    
            Out(ch,constrain(signal,0,HEMISPHERE_MAX_CV));
            
            
           
        }


        //-ghostils:Return to main menu after inactivity time out: 
        if(curMenuTimeOutTicks >= HEM_EG_UI_MENU_TIMEOUT_TICKS){
          curMenuTimeOutTicks = 0;
          curMenu = HEM_EG_UI_MAIN_MENU;
        }else{
          curMenuTimeOutTicks++;
        }
      
    }

    void View() {
        gfxHeader(applet_name());            
            
        //-ghostils: Don't draw ADSR or Indicators while in MAIN MENU:
        switch(curMenu){

          case HEM_EG_UI_EG1_MENU:
            DrawIndicator();
            DrawADSR();
          break;
          
          case HEM_EG_UI_EG2_MENU:
            DrawIndicator();
            DrawADSR();
          break;

          case HEM_EG_UI_CV1_DEST_MENU:
            DrawMainMenu();
            DrawCV1ModDestMenu();
          break;

          case HEM_EG_UI_CV2_DEST_MENU:
            DrawMainMenu();
            DrawCV2ModDestMenu();
          break;

          case HEM_EG_UI_EG1_ENV_OUT_STR:
            DrawMainMenu();
            DrawEG1OutStrMenu();
            break;
         
          default:
            DrawMainMenu();
            break;
        }
        
    }

    void OnButtonPress() {

     //-ghostils:If we are pressing buttons we are not timing out reset the timer:
     curMenuTimeOutTicks = 0;
     
    //-ghostils: Menu selection:
    if(curMenu == HEM_EG_UI_MAIN_MENU){
      switch(curMenuItem){
        case EG1_Edit:
          curMenu = HEM_EG_UI_EG1_MENU;
          curEG = 0;
          edit_stage = HEM_EG_ATTACK;          
          break;

        case EG2_Edit:
          curMenu = HEM_EG_UI_EG2_MENU;
          curEG = 1;
          edit_stage = HEM_EG_ATTACK;
          break;

        case EG1_CV1_Mod_Dest:
          curMenu = HEM_EG_UI_CV1_DEST_MENU;
        break;

        case EG2_CV2_Mod_Dest:
          curMenu = HEM_EG_UI_CV2_DEST_MENU;
        break;

        case EG1_Env_Out_Strength: 
          curMenu = HEM_EG_UI_EG1_ENV_OUT_STR;
        break;
                
        default:
          curMenu = HEM_EG_UI_MAIN_MENU;
          break;
        }
        //-ghostils:Return after switch selection we don't want to impact other clicks if set immediately we may add a click to other down stream items here:
        return;
    }
      
     
      //if (++edit_stage > HEM_EG_RELEASE) {edit_stage = HEM_EG_ATTACK;}
      //-ghostils: Allow selecting stages ONLY if we are in the EG1/EG2 menu:
      if(curMenu == HEM_EG_UI_EG1_MENU || curMenu == HEM_EG_UI_EG2_MENU){
        if (++edit_stage > HEM_EG_RELEASE) {edit_stage = HEM_EG_ATTACK;}                                            
      }   
     
    }

    void OnEncoderMove(int direction) {
        //-ghostils:If we are twiddling knobs we are not timing out reset the timer
        curMenuTimeOutTicks = 0;                

        //-ghostils: Drive Basic Menu Navigation:
        if(direction == HEM_EG_UI_ENCODER_RIGHT && curMenu == HEM_EG_UI_MAIN_MENU){
          if(curMenuItem == HEM_EG_UI_LAST_MENU_ITEM){
              curMenuItem = HEM_EG_UI_FIRST_MENU_ITEM;
          }else{
            curMenuItem++;
          }
        }else if(direction == HEM_EG_UI_ENCODER_LEFT && curMenu == HEM_EG_UI_MAIN_MENU){
          if(curMenuItem == HEM_EG_UI_FIRST_MENU_ITEM){
              curMenuItem = HEM_EG_UI_LAST_MENU_ITEM;
          }else{
            curMenuItem--;
          }
        }
        
        //-ghostils:Reference curEG as the indexer to current ADSR when editing stages:     
        if(curMenu == HEM_EG_UI_EG1_MENU || curMenu == HEM_EG_UI_EG2_MENU) {
          int adsr[4] = {attack[curEG], decay[curEG], sustain[curEG], release[curEG]};
          adsr[edit_stage] = constrain(adsr[edit_stage] += direction, 1, HEM_EG_MAX_VALUE);
          attack[curEG] = adsr[HEM_EG_ATTACK];
          decay[curEG] = adsr[HEM_EG_DECAY];
          sustain[curEG] = adsr[HEM_EG_SUSTAIN];
          release[curEG] = adsr[HEM_EG_RELEASE];        
        }


        //-ghostils:CV1 Mod Destination: 
        if(direction == HEM_EG_UI_ENCODER_RIGHT && curMenu == HEM_EG_UI_CV1_DEST_MENU){
          if(curCV1ModDestItem == HEM_EG_UI_LAST_CV_MOD_DEST_ITEM){
            curCV1ModDestItem = HEM_EG_UI_FIRST_CV_MOD_DEST_ITEM;
            //-ghostils:clear existing destination before setting new:
            for(int i = 0; i < 4; i++) {cvModDestVal[0][i] = 0;}            
          }else{
            curCV1ModDestItem++;
            //-ghostils:clear existing destination before setting new:
            for(int i = 0; i < 4; i++) {cvModDestVal[0][i] = 0;}            
          }
                       
        } else if(direction == HEM_EG_UI_ENCODER_LEFT && curMenu == HEM_EG_UI_CV1_DEST_MENU) {
          if(curCV1ModDestItem == HEM_EG_UI_FIRST_CV_MOD_DEST_ITEM){
            //-ghostils:clear existing destination before setting new:
            curCV1ModDestItem = HEM_EG_UI_LAST_CV_MOD_DEST_ITEM;
            for(int i = 0; i < 4; i++) {cvModDestVal[0][i] = 0;}                       
          }else{
            curCV1ModDestItem--;
            //-ghostils:clear existing destination before setting new:
            for(int i = 0; i < 4; i++) {cvModDestVal[0][i] = 0;}            
          }             
        }

        //-ghostils:CV2 Mod Destination: 
        if(direction == HEM_EG_UI_ENCODER_RIGHT && curMenu == HEM_EG_UI_CV2_DEST_MENU){
          if(curCV2ModDestItem == HEM_EG_UI_LAST_CV_MOD_DEST_ITEM){
            curCV2ModDestItem = HEM_EG_UI_FIRST_CV_MOD_DEST_ITEM;
            //-ghostils:clear existing destination before setting new:
            for(int i = 0; i < 4; i++) {cvModDestVal[1][i] = 0;}            
          }else{
            curCV2ModDestItem++;
            //-ghostils:clear existing destination before setting new:
            for(int i = 0; i < 4; i++) {cvModDestVal[1][i] = 0;}            
          }
                       
        } else if(direction == HEM_EG_UI_ENCODER_LEFT && curMenu == HEM_EG_UI_CV2_DEST_MENU) {
          if(curCV2ModDestItem == HEM_EG_UI_FIRST_CV_MOD_DEST_ITEM){
            //-ghostils:clear existing destination before setting new:
            curCV2ModDestItem = HEM_EG_UI_LAST_CV_MOD_DEST_ITEM;
            for(int i = 0; i < 4; i++) {cvModDestVal[1][i] = 0;}                       
          }else{
            curCV2ModDestItem--;
            //-ghostils:clear existing destination before setting new:
            for(int i = 0; i < 4; i++) {cvModDestVal[1][i] = 0;}            
          }             
        }

        //-ghostils:EG1 Amp Output Strength:
        if(direction == HEM_EG_UI_ENCODER_RIGHT && curMenu == HEM_EG_UI_EG1_ENV_OUT_STR){
          if(curCVAMPStr[0] >= 100){           
            curCVAMPStr[0] = 0;
          }else{
            curCVAMPStr[0]++;            
          }                      
        } else if(direction == HEM_EG_UI_ENCODER_LEFT && curMenu == HEM_EG_UI_EG1_ENV_OUT_STR) {
          if(curCVAMPStr[0] <= 0){
            curCVAMPStr[0] = 100;
          }else{
            curCVAMPStr[0]--;
          }             
        }
    }

    uint32_t OnDataRequest() {
        //-ghostils:Update to use an array and snapshot the values using curEG as the index      
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, attack[curEG]);
        Pack(data, PackLocation {8,8}, decay[curEG]);
        Pack(data, PackLocation {16,8}, sustain[curEG]);
        Pack(data, PackLocation {24,8}, release[curEG]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        //-ghostils:Update to use an array and snapshot the values using curEG as the index
        attack[curEG] = Unpack(data, PackLocation {0,8});
        decay[curEG] = Unpack(data, PackLocation {8,8});
        sustain[curEG] = Unpack(data, PackLocation {16,8});
        release[curEG] = Unpack(data, PackLocation {24,8});

        if (attack[curEG] == 0) Start(); // If empty data, initialize
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        /*
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "Mod 1=Att 2=Rel";
        help[HEMISPHERE_HELP_OUTS] = "Amp A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "A/D/S/R";
        */

        //-ghostils:Update onboard help:
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1=Ch1 2=Ch2";
        help[HEMISPHERE_HELP_CVS] = "Mod 1=Rel 2=Rel";
        help[HEMISPHERE_HELP_OUTS] = "Amp A=Ch1 B=Ch2";
        help[HEMISPHERE_HELP_ENCODER] = "A/D/S/R";
    }
    
private:
    int edit_stage;
    int attack[2]; // Attack rate from 1-255 where 1 is fast
    int decay[2]; // Decay rate from 1-255 where 1 is fast
    int sustain[2]; // Sustain level from 1-255 where 1 is low
    int release[2]; // Release rate from 1-255 where 1 is fast

    //-ghostils:TODO Modify to adjust independently for each envelope, we won't be able to do both Attack and Release simultaneously so we either have to build a menu or just do Release.
    //-Parameterize 
    
    int attack_mod; // Modification to attack from CV1    
    int release_mod[2]; // Modification to release from CV2
    int cv1;
    int cv2;
    int cvModDestVal[2][4];
    
    //-ghostils:Additions for tracking multiple ADSR's in each Hemisphere:
    int curEG;

    //-ghostils:UI Elements: 
    menuItems_t appMenu;
    int curMenu;
    int curMenuItem;
    int curMenuTimeOutTicks;
    //-X,Y coords for the menu highlight functionality:
    int menuX[10] = {0,0,0,0,0,38,38,38,38,38}; 
    int menuY[10] = {14,22,30,38,46,14,22,30,38,46}; 

    char *curCVModDestStr[4] = {"ATK","DEC","SUS","REL"};
    int curCV1ModDest;
    int curCV1ModDestItem;
    int curCV2ModDest;
    int curCV2ModDestItem;
    int curCVAMPStr[2];
    cvModDest_t cvModDest;
    
     
    // Stage management
    int stage[2]; // The current ASDR stage of the current envelope
    int stage_ticks[2]; // Current number of ticks into the current stage
    bool gated[2]; // Gate was on in last tick
    simfloat amplitude[2]; // Amplitude of the envelope at the current position

    int GetAmplitudeOf(int ch) {
        return simfloat2int(amplitude[ch]);
    }

    void DrawIndicator() {
        ForEachChannel(ch)
        {
            int w = Proportion(GetAmplitudeOf(ch), HEMISPHERE_MAX_CV, 62);
            //-ghostils:Update to make smaller to allow for additional information on the screen:
            //gfxRect(0, 15 + (ch * 10), w, 6);
            gfxRect(0, 15 + (ch * 3), w, 2);
        }

        //-ghostils:Indicate which ADSR envelope we are selected on:
        if(curEG == 0) {          
          gfxPrint(0,22,"EG1");
          gfxInvert(0,21,24,9);
        }else{
          gfxPrint(0,22,"EG2");
          gfxInvert(0,21,24,9);
        }      
    }

    void DrawADSR() {
        int length = attack[curEG] + decay[curEG] + release[curEG] + HEM_SUSTAIN_CONST; // Sustain is constant because it's a level
        int x = 0;
        x = DrawAttack(x, length);
        x = DrawDecay(x, length);
        x = DrawSustain(x, length);
        DrawRelease(x, length);
    }

    int DrawAttack(int x, int length) {
        //-ghostils:Update to reference curEG:
        int xA = x + Proportion(attack[curEG], length, 62);
        gfxLine(x, BottomAlign(0), xA, BottomAlign(HEM_EG_DISPLAY_HEIGHT), edit_stage != HEM_EG_ATTACK);
        return xA;
    }

    int DrawDecay(int x, int length) {
        //-ghostils:Update to reference curEG:
        int xD = x + Proportion(decay[curEG], length, 62);
        if (xD < 0) xD = 0;
        int yS = Proportion(sustain[curEG], HEM_EG_MAX_VALUE, HEM_EG_DISPLAY_HEIGHT);
        gfxLine(x, BottomAlign(HEM_EG_DISPLAY_HEIGHT), xD, BottomAlign(yS), edit_stage != HEM_EG_DECAY);
        return xD;
    }

    int DrawSustain(int x, int length) {
        int xS = x + Proportion(HEM_SUSTAIN_CONST, length, 62);
        //-ghostils:Update to reference curEG:
        int yS = Proportion(sustain[curEG], HEM_EG_MAX_VALUE, HEM_EG_DISPLAY_HEIGHT);
        if (yS < 0) yS = 0;
        if (xS < 0) xS = 0;
        gfxLine(x, BottomAlign(yS), xS, BottomAlign(yS), edit_stage != HEM_EG_SUSTAIN);
        return xS;
    }

    int DrawRelease(int x, int length) {
        //-ghostils:Update to reference curEG:
        int xR = x + Proportion(release[curEG], length, 62);
        int yS = Proportion(sustain[curEG], HEM_EG_MAX_VALUE, HEM_EG_DISPLAY_HEIGHT);
        gfxLine(x, BottomAlign(yS), xR, BottomAlign(0), edit_stage != HEM_EG_RELEASE);
        return xR;
    }


    //-ghostils:Draw Main Menu: 
    void DrawMainMenu(){
      gfxPrint(0,14,"EG1");
      gfxPrint(0,22,"CV1");      
      gfxPrint(0,30,"MOD");
      gfxPrint(0,38,"TYP");
      gfxPrint(0,46,"STR");      
      gfxPrint(38,14,"EG2");
      gfxPrint(38,22,"CV2");
      gfxPrint(38,30,"MOD");
      gfxPrint(38,38,"TYP");
      gfxPrint(38,46,"STR");
      //gfxPrint(38,54,"");

      //-Center divider:
      gfxLine(28,14,28,54);
            
      //-ghostils: Highlight select menu item:      
      gfxInvert(menuX[curMenuItem],menuY[curMenuItem],HEM_EG_UI_MENU_ITEM_WIDTH,HEM_EG_UI_CHAR_HEIGHT);      
    }

    //-ghostils:Draw Sub Menus:
    void DrawCV1ModDestMenu(){            
      //-ghostils:Draw CV Mod Destination String:
      gfxPrint(0,54,curCVModDestStr[curCV1ModDestItem]);            
      //-ghostils:Highlight CV1 Mod Destintation Value:     
      gfxInvert(0,54,HEM_EG_UI_MENU_ITEM_WIDTH,HEM_EG_UI_CHAR_HEIGHT);      
    }

    void DrawCV2ModDestMenu(){            
      //-ghostils:Draw CV Mod Destination String:
      gfxPrint(38,54,curCVModDestStr[curCV2ModDestItem]);            
      //-ghostils:Highlight CV2 Mod Destintation Value:     
      gfxInvert(38,54,HEM_EG_UI_MENU_ITEM_WIDTH,HEM_EG_UI_CHAR_HEIGHT);      
    }

    void DrawEG1OutStrMenu(){            
      //-ghostils:Draw CV Mod Destination String:
      gfxPrint(0,54,curCVAMPStr[0]);            
      //-ghostils:Highlight CV1 Mod Destintation Value:     
      gfxInvert(0,54,HEM_EG_UI_MENU_ITEM_WIDTH,HEM_EG_UI_CHAR_HEIGHT);      
    }



    void AttackAmplitude(int ch) {
        //-ghostils:Update to reference current channel:
        //-Remove attack_mod CV:
        //int effective_attack = constrain(attack[ch] + attack_mod, 1, HEM_EG_MAX_VALUE);
        //int effective_attack = constrain(attack[ch], 1, HEM_EG_MAX_VALUE);
        int effective_attack = constrain(attack[ch] + cvModDestVal[ch][ATTACK], 1, HEM_EG_MAX_VALUE);
        int total_stage_ticks = Proportion(effective_attack, HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_AD);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        if (effective_attack == 1) ticks_remaining = 0;
        if (ticks_remaining <= 0) { // End of attack; move to decay
            stage[ch] = HEM_EG_DECAY;
            stage_ticks[ch] = 0;
            amplitude[ch] = int2simfloat(HEMISPHERE_MAX_CV);
        } else {
            simfloat amplitude_remaining = int2simfloat(HEMISPHERE_MAX_CV) - amplitude[ch];
            simfloat increase = amplitude_remaining / ticks_remaining;
            amplitude[ch] += increase;
        }
    }

    void DecayAmplitude(int ch) {
        //-ghostils:Update to reference current channel:
        //int total_stage_ticks = Proportion(decay[ch], HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_AD);
        int total_stage_ticks = Proportion(decay[ch] + cvModDestVal[ch][DECAY], HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_AD);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        simfloat amplitude_remaining = amplitude[ch] - int2simfloat(Proportion(sustain[ch], HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
        if (sustain[ch] == 1) ticks_remaining = 0;
        if (ticks_remaining <= 0) { // End of decay; move to sustain
            stage[ch] = HEM_EG_SUSTAIN;
            stage_ticks[ch] = 0;
            amplitude[ch] = int2simfloat(Proportion(sustain[ch], HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
        } else {
            simfloat decrease = amplitude_remaining / ticks_remaining;
            amplitude[ch] -= decrease;
        }
    }

    void SustainAmplitude(int ch) {
        //-ghostils:Update to reference current channel:cvModDestVal[ch][ATTACK]
        //amplitude[ch] = int2simfloat(Proportion(sustain[ch] - 1, HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
        amplitude[ch] = int2simfloat(Proportion((sustain[ch] + cvModDestVal[ch][SUSTAIN]) - 1, HEM_EG_MAX_VALUE, HEMISPHERE_MAX_CV));
    }

    void ReleaseAmplitude(int ch) {
        //-ghostils:Update to reference current channel:
        //-CV1 = ADSR A release MOD, CV2 = ADSR A release MOD
        //int effective_release = constrain(release[ch] + release_mod[ch], 1, HEM_EG_MAX_VALUE) - 1;
        int effective_release = constrain(release[ch] + cvModDestVal[ch][RELEASE], 1, HEM_EG_MAX_VALUE) - 1;
        int total_stage_ticks = Proportion(effective_release, HEM_EG_MAX_VALUE, HEM_EG_MAX_TICKS_R);
        int ticks_remaining = total_stage_ticks - stage_ticks[ch];
        if (effective_release == 0) ticks_remaining = 0;
        if (ticks_remaining <= 0 || amplitude[ch] <= 0) { // End of release; turn off envelope
            stage[ch] = HEM_EG_NO_STAGE;
            stage_ticks[ch] = 0;
            amplitude[ch] = 0;
        } else {
            simfloat decrease = amplitude[ch] / ticks_remaining;
            amplitude[ch] -= decrease;
        }
    }

    int get_modification_with_input(int in) {
        int mod = 0;
        mod = Proportion(DetentedIn(in), HEMISPHERE_MAX_CV, HEM_EG_MAX_VALUE / 2);
        return mod;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ADSREG,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ADSREGPLUS ADSREGPLUS_instance[2];

void ADSREGPLUS_Start(bool hemisphere) {
    ADSREGPLUS_instance[hemisphere].BaseStart(hemisphere);
}

void ADSREGPLUS_Controller(bool hemisphere, bool forwarding) {
    ADSREGPLUS_instance[hemisphere].BaseController(forwarding);
}

void ADSREGPLUS_View(bool hemisphere) {
    ADSREGPLUS_instance[hemisphere].BaseView();
}

void ADSREGPLUS_OnButtonPress(bool hemisphere) {
    ADSREGPLUS_instance[hemisphere].OnButtonPress();
}

void ADSREGPLUS_OnEncoderMove(bool hemisphere, int direction) {
    ADSREGPLUS_instance[hemisphere].OnEncoderMove(direction);
}

void ADSREGPLUS_ToggleHelpScreen(bool hemisphere) {
    ADSREGPLUS_instance[hemisphere].HelpScreen();
}

uint32_t ADSREGPLUS_OnDataRequest(bool hemisphere) {
    return ADSREGPLUS_instance[hemisphere].OnDataRequest();
}

void ADSREGPLUS_OnDataReceive(bool hemisphere, uint32_t data) {
    ADSREGPLUS_instance[hemisphere].OnDataReceive(data);
}
