Hemisphere Suite: Alternate-Alternate Firmware for Ornament and Crime
===

### goals for this fork:
2/5/2023
<ul>  
  <li>Make some usability tweaks to specific applets.</li>
  <li>Roll some new ones from scratch.</li>
</ul>
-ghostils

### updates:
2/10/2023 - HEM_ADSREGPLUS.ino
<ul>
<li> Added internal modulation source destination for EG1/EG2 for Attack,Decay Sustain,Release,Envelope output strength. </li> 
<li> Positive modulation supported by clock sub division over steps equal to the max clock divisor or lower, ie divide by 2 and you can apply x modulation value for up to 2 steps. </li> 
<li> TODO: </li> 
<ul>
  <li>Probability as the additional internal modulation source. </li> 
  <li>Support for bi-polar modulation internally.</li>
  <li>Support for Sample and Hold as an internal modulation source based on incoming CV1/CV2 or specific active envelope stage for selected envelope.</li>
  <li>Move envelope triggered UI element to main menu screen for better visibility on active triggers/gates. </li>
</ul>
</ul>

2/7/2023 - HEM_ADSREGPLUS.ino
<ul>
<li> Added simple one page menu for EG1/EG2, press encoder on item to edit item in place. Timeout timer returns you to main menu for ALL sub menus/items after value is set. Moving or clicking encoder will reset timeout timer so you may continue editing a parameter.</li>
<li> EG1/EG2 edit buttons functional - Press encoder button on EG1/EG2 and you edit the envelop visually (reuses HEM_ADSREG.ino code).</li>
<li> EG1/EG2 CV1/CV2 destintation routing functional to Attack,Decay,Sustain,Release parameters CV1->EG1,CV2-EG2.</li> 
<li> EG1/EG2 Envelope output strength attenuation is functional. </li> 
<li> EG1/EG2 Envelope output strength available as a CV modulation destination </li>
</ul>


2/6/2023 - App Removal for more code space added new ADSREGPLUS Applet base for testing. 
<ul>
<li> REMOVED - Darkest Timeline </li>
<li> REMOVED - Neural Net </li>
<li> REMOVED - Pong </li> 
</ul>

2/5/2023 - HEM_ADSREG.ino
<ul>
<li>Envelopes are now editable independently giving up to 2 unique/triggerable ADSR's per hemisphere.</li>  
<li>CV modulation is now limited to the release stage for each channel.</li>
<li>Switching between envelopes is currently handled by simply pressing the encoder button until you pass the release stage on each envelope which will toggle the active envelope you are editing.</li>
<li>Active envelope you are editing is indicated by A or B just above the ADSR segments.</li>
</ul>


### firmware:

Hemisphere Suite is an open-source project by Jason Justian (aka chysn)

ornament**s** & crime**s** is a collaborative project by Patrick Dowling (aka pld), mxmxmx and Tim Churches (aka bennelong.bicyclist) (though mostly by pld and bennelong.bicyclist). it **(considerably) extends** the original firmware for the o_C / ASR eurorack module, designed by mxmxmx.

### hardware:

eurorack / teensy 3.2 DAC8565 quad 16bit CV module w/ OLED display

14HP, depth ~ 25mm

build guide: http://ornament-and-cri.me/

