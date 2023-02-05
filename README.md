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

