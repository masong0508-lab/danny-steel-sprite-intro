# Streamlined encounters and scene flow

The native subtitle header shows exact page progress and contextual input, rather than claiming every final action is merely NEXT. Intro ends with WALK; choice scenes with CHOOSE; neighborhood outcomes with SAVE; the promise with PROMISE; music sharing with SHARE; the performance with FINISH. Cancel is explicit when dialogue has a pending commitment. World camera HUDs share the same controls, show risk options only when available, and expose revisit versus notes.

The risk menu identifies the selected action before confirmation. Orchard encounter placards explain the tactic and lane/parry mechanic in plain language instead of unexplained timing fractions. Some journal and transition lines are rewritten to make the next step clearer. The bus menu describes the actual chapter transition.

A brief hardware brightness easing begins only when a completed new scene or illustration page becomes visible in VBlank. Six levels recover over six VBlanks, without waiting on rendering, pausing input, replaying transitions on each dialogue line, or changing animation clocks. The timed opening, active combat, and musical endings stay at full brightness. No extra framebuffer, save schema, or campaign flags. Regression additions exercise page-aware prompts and VBlank publication/clear behavior. Emulator capture and full cartridge CI remain required.
