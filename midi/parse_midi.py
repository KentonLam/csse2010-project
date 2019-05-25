import mido 
from icecream import ic

from collections import namedtuple

# notes from http://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies
note_names = {}
with open('midi_notes.csv', encoding='utf-8') as f:
    for l in f:
        num, name = l.strip().split(',')
        name = name.split(' ')[0].split('/')[0].replace('#', 'S')
        num = int(num)
        if 23 <= num <= 111: # limits on what the arduino can produce
            note_names[num] = 'NOTE_'+name
# print(note_names)
# input()
note_names[255] = 'NOTE_SILENCE'
note_names[254] = 'NOTE_SILENCE'

r = lambda x: round(x, 4)
GLOBAL_BPM = 160*3

def extract_notes(filename, start=None, stop=None, shift=0):
    mid = mido.MidiFile(filename)
    cur_time = 0
    tick_time = (1000*15/GLOBAL_BPM) / 1000
    cur_note = 0
    on_time = 0
    off_time = 0
    Note = namedtuple('Note', 'note duration')
    make_note = lambda n, d: Note(n, int(round(d)))
    notes = []
    ppq = (mid.ticks_per_beat )
    for msg in mid:
        if hasattr(msg, 'time'):
            cur_time += msg.time
        if start and cur_time < start:
            continue 
        if stop and cur_time > stop:
            break
        # print(msg)
        if msg.type.startswith('note_'):
            
            if msg.channel not in (0, 2, 5): continue
            # print(msg)
            if msg.type == 'note_on' and msg.velocity != 0:
                if not cur_note:
                    cur_note = msg.note
                    on_time = cur_time
                    if on_time != off_time:
                        if notes:
                            ic('silence', (on_time-off_time)/tick_time)
                            if int(round((on_time-off_time)/tick_time)):
                                notes.append(make_note(255, (on_time-off_time)/tick_time))
                elif msg.note != cur_note:
                    ic('OVERLAPPING NOTES AT', cur_time, cur_time*(GLOBAL_BPM*ppq/60))
                    
                    break
            elif msg.type == 'note_off' or msg.velocity == 0:
                if msg.note == cur_note:
                    off_time = cur_time
                    new_note = make_note(msg.note, (off_time-on_time)/tick_time)
                    if notes and new_note.note == notes[-1].note:
                        ...
                        notes.append(make_note(255, 1))
                    notes.append(new_note)
                    ic(str(len(notes)).rjust(3), 'Note:', (msg.note-60)*' ' + '*', msg.note,  ' END:', r(cur_time), ' START:', r(on_time), ' TICK:', r(on_time*GLOBAL_BPM*ppq/60), ' SEMIQUA:', r((cur_time-on_time)/tick_time))
                    cur_note = 0
                else:
                    ic(msg)
    print('MIDI:', filename)
    notes = [x for x in notes if x.duration]
    
    print()
    SHIFT = shift
    print(', '.join(
        f'NOTE_WORD({note_names[n+SHIFT] if n != 255 else "NOTE_SILENCE"}, {d})' 
        for n, d in notes))
            
    print()
    print((1000*15/GLOBAL_BPM), 'ms ticks')
    print(len(notes), 'distinct notes')
    print(len(notes)*2, 'bytes of progmem')
    print()
    print()
    print()
    print()

ic.disable()
extract_notes('unowen.mid', shift=0)
extract_notes('windows.mid', 3.5, 7, -12)
extract_notes('windows.mid', 7, 9.5, -12)
