LG D85530B

Touch sounds speaker
31: fb-protection
1: low-latency-playback
OUT: 2: speaker
acdb_id = 14
48000

Touch sounds headset
1: low-latency-playback
OUT: 4: headphones
acdb_id = 10
48000

Voice recording handset mic
7: audio-record
IN: handset-mic -> 140: voice-memo
acdb_id = 63

Voice recording headset mic
7: audio-record
IN: headset-mic -> 142: headset-voice-memo
acdb_id = 66

Music speaker
31: fb-protection
3: compress-offload-playback
OUT: 2: speaker
acdb_id = 14
44100

Music headset
3: compress-offload-playback
OUT: 4: headphones
acdb_id = 10
44100

Voice call handset
11: voice-call
OUT: 6: voice-handset
IN: 109: voice-dmic-ef
acdb_rx = 7, acdb_tx = 41

Voice call speaker
11: voice-call
OUT: 7: voice-speaker
IN: 165: voice-speaker-dmic-ef-nxp
acdb_rx = 14, acdb_tx = 43

Voice call headset
11: voice-call
OUT: 8: voice-headphones
IN: 102: voice-headset-mic
acdb_rx = 10, acdb_tx = 8

VOIP handset // TODO
<path_id>: <path_name>
OUT: <dev_id>: <dev_name>
IN: <dev_id>: <dev_name>
acdb_rx = <acdb_rx_id>, acdb_tx = <acdb_tx_id>
<sampling_rate>

VOIP headset // TODO
<path_id>: <path_name>
OUT: <dev_id>: <dev_name>
IN: <dev_id>: <dev_name>
acdb_rx = <acdb_rx_id>, acdb_tx = <acdb_tx_id>
<sampling_rate>
