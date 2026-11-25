#pragma once
#include "story.h"
/* All metadata is ROM-backed. Mode 0 dial, 1 timing, 2 pattern.
   One of each per location. Explicit local writing and reward type. */
enum { IN_WALK=0,IN_BRIEF=1,IN_GAME=2,IN_RETRY=3,IN_RESULT=4,IN_WRONG=5 };
enum { IN_DIAL=0,IN_TIMING=1,IN_PATTERN=2 };
enum { IN_REWARD_BOND=0,IN_REWARD_SPARK=1,IN_REWARD_WORK=2 };
typedef struct {
 const char *speaker,*intro,*order,*finish,*right,*left;
 const char *steps[3];
 uint8_t mode[3],value[3],reward;
} InteriorTask;
static const InteriorTask interior_tasks[14]={
 {"ELSIE","A loose seam is no end.","Sort, wash, fold it.","Clean clothes. A fresh start.","Elsie can reuse the towels.","The red coat will last.",{"SORT","WASH","FOLD"},{0,1,2},{2,5,6},0},
 {"PHOTOGRAPHER","A name behind the face.","Frame, focus, then print.","The print has its place.","Faces matter without names.","Your name stays with the print.",{"FRAME","FOCUS","PRINT"},{2,0,1},{4,3,9},1},
 {"DANNY","The skyline belongs here.","Measure, sketch, shade.","One more study for the roof.","A city rhythm, caught in ink.","The roof now has a study.",{"MEASURE","SKETCH","SHADE"},{0,2,1},{5,7,11},1},
 {"REPAIRMAN","Small faults, good music.","Check, tune, test the radio.","A radio sings again.","Useful parts are saved.","The speaker finds a voice.",{"CHECK","TUNE","TEST"},{2,0,1},{1,6,7},2},
 {"POST CLERK","A note before the bus.","Address, stamp, file it.","One more way to reach home.","Your words are still yours.","The letter is in the bag.",{"ADDRESS","STAMP","FILE"},{2,1,0},{2,8,4},0},
 {"NEIGHBOR","You need not disappear.","Find, write, shelve the note.","The old doorstep remembers.","Your note is safe on the shelf.","Someone knows you came back.",{"FIND","WRITE","SHELVE"},{0,2,1},{3,9,5},0},
 {"VENDOR","Help before the rain.","Sort, weigh, pack the fruit.","A crate is safe and dry.","Apples for the road ahead.","You helped close the stall.",{"SORT","WEIGH","PACK"},{2,1,0},{8,10,2},2},
 {"ARCHIVIST","A recording is a record.","Label, splice, sleeve the tape.","Your old work has a home.","The new sleeve protects it.","The rehearsal has a name.",{"LABEL","SPLICE","SLEEVE"},{0,1,2},{4,6,11},2},
 {"DANNY","The garden can wait.","Seed, water, plan a question.","A small plan for tomorrow.","Rest is part of the day.","The question is written down.",{"SEED","WATER","PLAN"},{2,1,0},{5,9,3},1},
 {"ELSIE","The suit paid the rent.","Clean, fold, sign out.","Good work. No grand speech.","You thanked the people here.","The next worker has a suit.",{"CLEAN","FOLD","SIGN OUT"},{1,2,0},{6,4,5},2},
 {"OLD MAN","A bench for two people.","Repair, paint, share a seat.","A dry place for a neighbor.","Quiet company is enough.","There's room for both of us.",{"REPAIR","PAINT","SHARE"},{0,1,2},{6,10,3},0},
 {"DANNY","A voice through the wire.","List, dial, send the message.","Someone can hear from you.","Your message is on its way.","You made the call.",{"LIST","DIAL","MESSAGE"},{2,0,1},{7,5,9},0},
 {"DANNY","Keep the rail and water.","Sort, clean, sketch the view.","A cleaner view for the canvas.","The canal is a little cleaner.","A new line for your canvas.",{"SORT","CLEAN","SKETCH"},{2,1,0},{2,7,6},1},
 {"DRIVER","The last bus still runs.","Check, mark, post the route.","No guessing in the rain.","The bench has a free seat.","The late route is marked.",{"CHECK","MARK","POST"},{0,2,1},{2,10,11},0},
};
static inline unsigned interior_step(const Story *s){return s->activity_choice<3u?s->activity_choice:2u;}
static inline unsigned interior_mode(unsigned id,unsigned step){return interior_tasks[id].mode[step];}
static inline unsigned interior_dial_target(unsigned id,unsigned step){return interior_tasks[id].value[step]&7u;}
static inline unsigned interior_timing_target(unsigned id,unsigned step){return 3u+interior_tasks[id].value[step]%10u;}
static inline unsigned interior_timing_position(const Story *s){
 unsigned p=(s->cinema_frame/4u)%30u;
 return p<16u?p:30u-p;
}
static inline unsigned interior_pattern(unsigned id,unsigned step,unsigned i){
 /* Three authored, deterministic directions per workstation. */
 return (interior_tasks[id].value[step]+id+i*(1u+(step&1u)))&3u;
}
