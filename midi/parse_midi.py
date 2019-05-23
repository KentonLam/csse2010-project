import mido 

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

mid = mido.MidiFile('necro.mid')
cur_time = 0
tick_time = 46.87500/1000
cur_note = 0
on_time = 0
off_time = 0
Note = namedtuple('Note', 'note duration')
make_note = lambda n, d: Note(n, int(round(d)))
notes = []
ppq = (mid.ticks_per_beat )
bpm = 160
for msg in mid:
    if hasattr(msg, 'time'):
        cur_time += msg.time
    print(msg)
    if msg.type.startswith('note_'):
        
        if msg.channel not in (0, 2, 5): continue
        # print(msg)
        if msg.type == 'note_on':
            if not cur_note:
                cur_note = msg.note
                on_time = cur_time
                if on_time != off_time:
                    if notes:
                        print('PRECEDING SILENCE of semiquaver length', (on_time-off_time)/tick_time)
                        notes.append(make_note(255, (on_time-off_time)/tick_time))
            else:
                print('OVERLAPPING NOTES AT', cur_time)
                break
        elif msg.type == 'note_off':
            if msg.note == cur_note:
                off_time = cur_time
                new_note = make_note(msg.note, (off_time-on_time)/tick_time)
                if notes and new_note.note == notes[-1].note:
                    ...
                    # notes.append(make_note(255, 1))
                notes.append(new_note)
                print(str(len(notes)).rjust(3), 'Note:', (msg.note-70)*' ' + '*', msg.note,  ' END:', r(cur_time), ' START:', r(on_time), ' TICK:', r(on_time*bpm*ppq/60), ' SEMIQUA:', r((cur_time-on_time)/tick_time))
                cur_note = 0
            else:
                print(msg)

notes = [x for x in notes if x.duration]

# print(notes)
print()
SHIFT = -0
print(', '.join(
    f'NOTE_WORD({note_names[n+SHIFT] if n != 255 else "NOTE_SILENCE"}, {d})' 
    for n, d in notes))
        
print()
print(len(notes), 'distinct notes')
print(len(notes)*2, 'bytes of progmem')