# Danny Steele — source-timed cutscenes and presentation

Base: reference-likeness PR #54 (290f50b4508da9f25e94224bc1a6c736324659c8). No change to measured source poses, reference art, story or main.

## Design and implementation
- Use coordinate-based nearest rounding to remove the direction bias between traced keyframes. Keep the generated header and generator byte-equivalent.
- When any optional traced matte is available, use its held timestamp for procedural anatomy, props, contact shadows and floor depth; retain smooth source-only fallback.
- Preserve the opening's exact 240×135 crop and neighborhood panel layout. Reclaim 19 rows of native cinema from the old dialogue card with centered captions and visible controls. Ordinary world dialogue retains its existing card.
- Extend the existing source-clock and graphics tests; run the project's native tests and build before integrating. This branch is stacked on the unmerged likeness work rather than overwriting it. No source-reference media is added.
