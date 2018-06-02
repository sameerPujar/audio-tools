adb root
adb remount

adb shell 'tinymix -D 1 "I2S1 Mux" "MIXER1-1"'
adb shell 'tinymix -D 1 "MIXER1-1 Mux" "ADMAIF1"'
adb shell 'tinymix -D 1 "Adder1 RX1" "1"'
adb shell 'tinymix -D 1 "Mixer Enable" "1"'
adb shell 'tinymix -D 1 "ADMAIF1 Mux" "I2S1"'

#playback controls
adb shell 'tinymix -D 1 "x Headphone Playback Volume" "0x1e"'
adb shell 'tinymix -D 1 "x Stereo DAC MIXR DAC R1 Switch" "1"'
adb shell 'tinymix -D 1 "x Stereo DAC MIXL DAC L1 Switch" "1"'
adb shell 'tinymix -D 1 "x HPO R Playback Switch" "1"'
adb shell 'tinymix -D 1 "x HPO L Playback Switch" "1"'
adb shell 'tinymix -D 1 "x Headphone Jack Switch" "1"'

#capture controls
adb shell 'tinymix -D 1 "x RECMIX1L BST1 Switch" "1"'
adb shell 'tinymix -D 1 "x RECMIX1R BST1 Switch" "1"'
adb shell 'tinymix -D 1 "x Stereo1 ADC Source" "ADC1"'
adb shell 'tinymix -D 1 "x Stereo1 ADC1 Source" "ADC"'
adb shell 'tinymix -D 1 "x Stereo1 ADC MIXL ADC1 Switch" "1"'
adb shell 'tinymix -D 1 "x Stereo1 ADC MIXR ADC1 Switch" "1"'
adb shell 'tinymix -D 1 "x TDM Data Mux" "AD1:AD2:DAC:NUL"'
adb shell 'tinymix -D 1 "x IN1 Boost Volume" 40'
adb shell 'tinymix -D 1 "x Mic Jack Switch" "1"'

adb push $1 /sdcard/Music/
adb shell tinycap /sdcard/Music/$2 -D 1 -d 0 -r 48000 -c 2 -b 16&
adb shell tinyplay /sdcard/Music/$1 -D 1 -d 0&
sleep 10
adb shell 'tinymix -D 1 "ADMAIF1 Mux" "None"'
sleep 50
