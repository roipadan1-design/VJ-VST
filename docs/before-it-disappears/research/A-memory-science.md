# Strand A: the science of memory as visual algorithms

*For "Before It Disappears". Research strand A of the art-direction brief (sections 5A, 6, 7.6, 8).*

## How to read this

- **Source status.** Every source is marked **[search]**: I saw it only in a WebSearch result, as a title, a snippet and a URL. I tried WebFetch three times, on PubMed, PMC and the Journal of Neuroscience. The egress proxy blocked all three, so I stopped. **No source in this file was [opened].** Claims rest on the search snippets and on well-known textbook findings. Where a snippet was thin, I say so.
- **Inferences** are labelled *(inference)*. Numbers for passes, buffer sizes and time constants are my design proposals, not measurements.
- **GPU cost** is for the Intel Iris Xe. **S** means under about 0.3 ms per frame, or work that runs only on an event. **M** means one to three extra quarter-resolution passes every frame, or more than 10 MB of extra VRAM. **L** means extra full-resolution passes every frame. I follow the budget in `docs/RESEARCH-REPORT.md` §B7 (at most 6 full-resolution passes, persistent buffers at half or quarter resolution) and do not repeat it here.
- **Relation to earlier work.** `RESEARCH-REPORT.md` §B5 already proposes a ghost texture on scene change (τ ≈ 1 bar). Its scene concept #15 ("Ghost Crossfade Scene") is related. The ideas below go further. They add a **long-lived show memory** that survives scene switches and is **rewritten each time it is recalled**.

## What exists in the engine (checked in the code)

- **Trails** (`engine/Source/LookPass.cpp`, `trailSource`). The trail pass computes `max(scene, memory*retain - 0.002)`, with a slight zoom-in ("push"). Its time constant τ runs from 0.03 to 0.48 s. It lives in a ping-pong `history` buffer at internal resolution.
  - `PresetManager.cpp:234` calls `clearTrails()` on **every scene switch**. So today the engine only has perceptual persistence, lasting under a second. It has no episodic memory.
- **The film chain** (`LookPass.h`) runs in this order: weave → smear/glitch → fringing → acutance → grain → flicker → crush → palette gradient map (3 colours on luminance) → lifted blacks → halation → symbols → dust → flash → dither.
  - Halation already uses a **quarter-resolution ping-pong** (`halo[2]`). Quarter-resolution buffers are therefore an established pattern.
- **Signals** (`engine/Source/Signals.h`):
  - `presence` (smoothed over about 2.5 s) and `build` (rises in about 1.5 s, releases in about 4 s)
  - `flux`, `centroid`, `flatness`, and a 32-band `spectrum`
  - events: kick, snare, hat, `userTrigger` (the manual HIT) and `midiNote`
  - `Clock::crossedBar()`
- **Modulation** (`Modulation.h`) supplies envelopes, LFOs and routes that can read `presence` and `build`.

**Proposed home for the new work** *(inference)*: a new class `MemoryBank`, owned next to `LookPass`.
- **Capture.** Blit the scene texture, before the film chain, down to quarter resolution with one linear `glBlitFramebuffer`.
- **Compositing.** Add samplers and uniforms to LookPass's **final shader**, placed **before crush and palette**. Recalled memories then pass through the same grain, palette and dust as the live image, and belong to the same world.
- **Survival.** `clearTrails()` must not touch the bank. Its survival across scenes is the point.
- **Rewriting.** A write-back ("reconsolidation") pass runs at quarter resolution **only when a recall happens**, not every frame.
- **Base storage cost.** Eight slots of 480×270 R8 luminance take about 130 KB each, about 1 MB in total. The cost is **S**.

---

## The ideas

### 1. Reconsolidation: recall rewrites the stored image

1. **Finding.** A consolidated fear memory becomes labile again when it is reactivated. Blocking protein synthesis in the amygdala right after reactivation produced amnesia (Nader, Schafe & LeDoux 2000). [search] https://www.nature.com/articles/35021052 · https://pubmed.ncbi.nlm.nih.gov/10963596/
   - **Human, visual evidence.** On day 3, people placed objects closer to the *wrong* locations they had recalled on day 2 than to the original locations (Bridge & Paller 2012). [search] https://www.jneurosci.org/content/32/35/12144 · https://www.psypost.org/each-time-you-recall-an-event-your-brain-distorts-it/
2. **Process.** Each retrieval reopens the trace, and whatever happened during the retrieval gets written back.
3. **Visual behaviour.** A captured image never comes back the same twice. The *current* recall is always a copy of the *previous recall*, not of the original.
4. **Engine sketch.** On `recall(slot)`, run one quarter-resolution pass that reads `mem[slot]` and writes `mem'[slot]`:
   - a small blur, with radius 1 + 0.3·n texels, where n is the recall count
   - quantisation to `max(3, 16 − 2n)` levels
   - a displacement step toward an attractor field (see idea 3)
   - a mix with the downsampled live scene at weight β (see idea 2)

   The count n is stored per slot. A deterministic seed `hash(slot, n)` makes recall #3 degrade the same way at every show. Cost: **S**, one quarter-resolution pass per recall event plus one extra texture fetch in the final shader.
5. **Text.** *"The story is rewritten / every time it is remembered."* And, almost literally what Bridge & Paller measured: *"Perhaps we remember / the last time / we remembered it."*

### 2. The prediction-error gate: only surprise reopens a memory

1. **Finding.** Reactivation made a human fear memory labile *only* when retrieval involved a prediction error, a mismatch between what was expected and what happened (Sevenster, Beckers & Kindt 2013). [search] https://pubmed.ncbi.nlm.nih.gov/23413355/ · https://science.sciencemag.org/content/339/6121/830.editor-summary
2. **Process.** Recall alone is not enough. The memory is rewritten only when the present fails to match it.
3. **Visual behaviour.** A memory recalled over a matching moment comes back almost intact. A memory recalled over a *different* moment (another scene, another palette, a sudden hit) is visibly rewritten by that moment.
4. **Engine sketch.** Set the write-back weight to `β = 0.02 + 0.2·PE`. PE is the mean absolute difference between the stored slot and the live quarter-resolution frame. Read it from the 1×1 top mip of a difference texture: one `glGenerateMipmap` on a 480×270 target, or a 4-tap reduction chain. PE can also add `flux` at the moment of recall. Cost: **S**.
   - **Why it helps us.** Rewriting becomes a **rare, correlated event**, which is the owner's taste, instead of a constant wash.
5. **Text.** *"And then, / something changes."* This gate is how the piece moves from recalling to rewriting.

### 3. Serial reproduction: every memory drifts toward the same shape

1. **Findings.**
   - **Bartlett (1932).** In serial and repeated reproduction of *The War of the Ghosts*, the story grew shorter. Odd details were dropped or "rationalised" into familiar forms. [search] https://www.tandfonline.com/doi/full/10.1080/09658211.2022.2059514 · https://www.sciencedirect.com/science/article/abs/pii/S2211368114000485
   - **Replication caveat.** Bergman & Roediger (1999) asked whether the repeated-reproduction results replicate, and reported partial replication. [search] https://link.springer.com/article/10.3758/BF03201224
   - **The same effect, made visual.** Langlois, Jacoby, Suchow & Griffiths (2021) ran telephone-game chains of *dot positions*, 20 iterations each, across 85 experiments with 9,202 participants. Positions that started uniform collapsed onto attractors: centres of mass, quadrant centres of a circle, and the medial axis ("shape skeleton") of shapes. Chains converge to the observers' prior. [search] https://www.pnas.org/doi/10.1073/pnas.2012938118 · https://cocosci.princeton.edu/papers/langloisserial.pdf
2. **Process.** Each reproduction pulls the content a small step toward what the mind expects, so iterated copies converge on the prior.
3. **Visual behaviour.**
   - Different memories, recalled again and again, **stop being different**.
   - Their forms slide toward one shared shape: a skeleton line, a centre, a horizon.
   - By the end, every memory in the show has become the same mark.
4. **Engine sketch.**
   - **Prior field.** A static 480×270 R8 distance-field texture P holds the show's "prior", for example a thin vertical seam of light or a low horizon. It must **not** be a pair of dots (no eyes).
   - **Each recall.** Resample the memory at `uv − α·∇P·(1 − P)`, with α ≈ 0.006 per recall. Blend by `0.1·n` toward `P` itself.
   - **Result.** After roughly 12–20 recalls every slot is mostly P. This matches Langlois's 20 iterations *(inference: the numbers were chosen to match, not derived)*.
   - Cost: **S** (it folds into idea 1's pass).
5. **Text.** It serves the ending: *"Only a mark. / The quiet mark / left / by everything / that ever / moved us."* The shared attractor **is** the mark. It also serves *"we no longer carry the memory. / The memory carries us."*

### 4. Gist and verbatim traces, and the outline that survives

1. **Findings.**
   - **Fuzzy-trace theory.** Memory stores a verbatim trace (surface detail) and a gist trace (meaning) in parallel. The two survive at different rates: verbatim traces are typically lost within days, while gist persists (Brainerd & Reyna). [search] https://pubmed.ncbi.nlm.nih.gov/11605365/ · https://journals.sagepub.com/doi/10.1111/1467-8721.00192
   - **Transformation hypothesis.** Over time, context-rich episodic memories are transformed into gist-like or schematic versions (Winocur & Moscovitch 2011). [search] https://pubmed.ncbi.nlm.nih.gov/21729403/
2. **Process.** At encoding, split the image into a fast-decaying detail band and a slow-decaying structure band.
3. **Visual behaviour.** A recalled image first loses its texture and fine grain, then its interior tones. The **outline and the large masses stay longest**.
4. **Engine sketch.** At capture, store three textures:
   - **gist:** a 240×135 blurred copy
   - **verbatim:** `mem − upsample(gist)`, signed, RG8 or R16F
   - **edge:** a Sobel magnitude, R8

   Display `gist·g(t) + verbatim·v(t) + edge·e(t)`. The weights follow the power-law curves of idea 8:
   - v: t₀ = 4 bars, b = 1.2
   - g: t₀ = 90 s, b = 0.4
   - e: t₀ = 5 min, b = 0.2

   All three are uniforms only. No extra pass runs per frame; the split is done once at capture. Cost: **S**, about 1.3 MB per slot. This is the brief's "edge-only memory" seed, with a scientific backbone.
5. **Text.** *"It keeps the outline, / never the whole story. / The edges begin to soften."*
   - **Design note.** The text says the *edges* soften. So `e(t)` should also blur slowly: an edge-blur radius growing with recall count. The outline survives, but softer.

### 5. Boundary extension: memory invents what lay beyond the frame

1. **Findings.**
   - After seeing close-up photographs for 15 s each, 95% of participants' drawings included scene content that had lain just outside the frame (Intraub & Richardson 1989). [search] https://pubmed.ncbi.nlm.nih.gov/2522508/ · http://www.scholarpedia.org/article/Boundary_extension
   - The extension appears after as little as a **42 ms** interruption (Intraub & Dickinson 2008, "False memory 1/20th of a second later"). [search] https://www.ncbi.nlm.nih.gov/pubmed/19000211
2. **Process.** The mind remembers the expected surround as if it had been seen: memory extrapolates beyond the edge of the view.
3. **Visual behaviour.**
   - A recalled frame shows up **smaller than the screen**, with a visible border.
   - The region *outside* the original border fills with plausible, generated continuation that never existed.
   - With each recall the frame shrinks a little more and the invented surround grows.
   - Because the effect is immediate (42 ms), the extension should appear **the instant** the memory surfaces, not fade in.
4. **Engine sketch.**
   - **Display.** Draw the slot at scale `s = 0.94 − 0.03·n`, with a minimum of 0.6, centred.
   - **Outside the rect.** Sample the slot with mirrored UVs through a 3-octave fbm domain warp. Take the result from a low mip, so it is blurrier and less detailed. Attenuate it by distance from the border (falloff about 0.15 of the screen width).
   - **Option.** Let the *live* generator fill the periphery, so the present invents the past's surround.
   - This is the opposite of Trails' zoom-in "push". Memory pulls **out**.
   - Cost: **S**, 3–4 extra fetches plus fbm at quarter-resolution sample points in the final shader.
5. **Text.** This is the pivot, almost word for word: *"A memory reaches / the edge of itself. / Beyond that, / it no longer recalls. / It creates."* It also serves *"A memory lives inside a frame."*
   - **Related finding.** Remote and self-conscious memories are more often seen from outside, in the "observer" perspective (Nigro & Neisser 1983). [search] https://www.sciencedirect.com/science/article/abs/pii/0010028583900166
   - Shrinking the frame over successive recalls is a way to **see the moment from farther away** each time.

### 6. The clarity paradox: the oldest memories feel the clearest

1. **Findings.**
   - **Flashbulb memories.** Students recorded their memories of 9/11 on 12 September 2001. Consistency declined over 1, 6 or 32 weeks, just as it did for everyday memories. But vividness and belief in accuracy stayed high only for the flashbulb memories (Talarico & Rubin 2003): confidence without accuracy. [search] https://journals.sagepub.com/doi/abs/10.1111/1467-9280.02453 · https://sites.lafayette.edu/talaricj/files/2009/09/TalaricoRubin2003.pdf
   - **Reminiscence bump.** Older adults recall a disproportionate number of memories from ages 10 to 30 (Rubin, Wetzler & Nebes 1986). [search] https://pmc.ncbi.nlm.nih.gov/articles/PMC6841349/
   - One account ties the bump to life-story and identity formation (Glück & Bluck 2007). [search] https://lifestorylab.psych.ufl.edu/wp-content/uploads/sites/84/Bump.Glueck.Bluck_.2007.pdf
2. **Process.** Rehearsal makes a memory *simpler and more confident*, not blurrier. It is rewritten into a cleaner schema that feels sharp.
3. **Visual behaviour.** The *most recalled* slot is the **crispest** image in the show: high contrast, few tones, hard graphic edges. It is also the **least faithful**, displaced toward the attractor. Blur is how new memories degrade; hardening is how old ones do.
4. **Engine sketch.** In the reconsolidation pass (idea 1), after about the 5th recall, change the blur into an **unsharp mask** (amount `0.3·(n−4)`) plus an S-curve. Posterisation then converges toward the One Bit look: a hard two-tone. The existing LOOK *Crush* already has this vocabulary. Cost: **S**.
5. **Text.** *"Maybe that is why / the oldest memories / often feel the clearest. / Not because they remained untouched, / but because / they have been rewritten / by every version of ourselves / that has carried them."* The choreographer's explanation fits the flashbulb data remarkably well.

### 7. Involuntary memory: the unbidden return, cued by a sense

1. **Findings.**
   - **Frequency and conditions.** Involuntary autobiographical memories are common, reported around 22 per day with a button counter. They arise mostly under **diffuse attention**, cued by features of the situation, and they are more specific and more emotional than voluntary ones (Berntsen). [search] https://www.bps.org.uk/psychologist/involuntary-autobiographical-memories · https://pubmed.ncbi.nlm.nih.gov/15552356/ · https://pmc.ncbi.nlm.nih.gov/articles/PMC7741080/
   - **Cues are often not literal senses.** A diary study summarised in those results found more abstract or verbal cues (68%) than sensory ones (30%). *I could not open it to confirm which study this was: unknown.*
   - **Smell.** Odour-cued memories come from earlier in life, the first decade, than the usual bump (Chu & Downes 2000). [search] https://pubmed.ncbi.nlm.nih.gov/10668001/
   - **The model.** Proust's madeleine and lime-blossom tea (*Swann's Way*, 1913) is the literary model. [search] https://www.sparknotes.com/lit/swannsway/section1/
   - **Context dependence.** Recall is better in the context where learning happened (Godden & Baddeley 1975). A 2021 replication by Murre notes the effect is weak, around d ≈ 0.25. [search] https://royalsocietypublishing.org/rsos/article/8/11/200724/95735/The-Godden-and-Baddeley-1975-experiment-on-context
2. **Process.** A stored episode resurfaces by itself when the present partly matches its context, especially when nothing else holds the attention.
3. **Visual behaviour.**
   - During a sparse, quiet passage, an image from much earlier returns **unbidden** for a few seconds, because the sound resembles the sound of the moment it was captured.
   - It is never triggered in busy passages.
4. **Engine sketch.** All CPU, apart from one composite. Cost: **S**.
   - At capture, store a fingerprint: the 32-band `spectrum` averaged over the surrounding 4 bars, plus `centroid` and `flatness`.
   - Every frame, compute the cosine similarity between the live spectrum and every fingerprint. Fire the memory if all of these hold:
     - similarity > 0.92
     - `presence` < 0.3 (diffuse attention)
     - cooldown ≥ 90 s
     - at most one firing per section
   - The surfaced memory rises over 2 s, holds for 4–8 s and fades over 4 s.
   - The same audio in Live gives the same firings at every show, so the result is repeatable *(inference)*.
   - Add a single arm/disarm toggle on the controller. A manual trigger through `userTrigger` or a MIDI note covers the "familiar light" cue.
5. **Text.** *"It follows us quietly, / settling in places / we never expect. / A voice. / A scent. / A familiar light."*

### 8. The forgetting curve: memories wait, and do not reach zero

1. **Findings.**
   - Ebbinghaus's savings curve has been replicated: 70 hours of learning with retention intervals from 20 minutes to 31 days. The curve shows an upward **jump at 24 hours**, possibly an effect of sleep (Murre & Dros 2015). [search] https://journals.plos.org/plosone/article?id=10.1371%2Fjournal.pone.0120644
   - Forgetting is best described by a **power function** of time, better than an exponential (Wixted & Ebbesen 1991). [search] https://journals.sagepub.com/doi/10.1111/j.1467-9280.1991.tb00175.x
2. **Process.** Strength falls fast at first, then almost stops. A power law has a long tail, so an old trace is dormant rather than gone.
   - "Savings" means relearning is faster than first learning.
3. **Visual behaviour.**
   - Surfaced memories drop quickly to a **faint floor**, about 3–6% luminance, and **stay there** as a ghost in the blacks.
   - Each later recall rises *faster* than the previous one (savings).
   - After a long blackout (the "sleep"), stored memories come back slightly stronger *(inference, from the 24 h jump)*.
4. **Engine sketch.**
   - Strength `a·(1 + t/t₀)^−b`, with a floor. Rise time `2 s · 0.7ⁿ`.
   - After a BLACKOUT of 8 s or more, add +10% strength to all slots.
   - This is parameters only. Cost: **S** (zero GPU).
5. **Text.** It opens the piece: *"We think memories fade. / They don't. / They wait. / Quietly."*
   - The existing Trails uses an exponential (`exp(−dt/τ)`). The show memory should **not** copy that curve.

### 9. Generation loss: the copy of a copy, and the room's resonance

1. **Findings.**
   - **Generation loss.** Quality is lost between copies or transcodes. JPEG re-saving accumulates artefacts (the "photocopier effect"). Analogue dubs lose high frequencies, misalign luma and chroma, and add noise. [search] https://en.wikipedia.org/wiki/Generation_loss · https://uploadcare.com/blog/jpeg-quality-loss/
   - **Lucier.** *I Am Sitting in a Room* (1969) re-records speech through a room 32 times. The room's resonant frequencies are reinforced until the words dissolve into its resonance (the 1970 recording lasts about 40 minutes). [search] https://www.moma.org/explore/inside_out/2015/01/20/collecting-alvin-luciers-i-am-sitting-in-a-room/ · https://www.dramonline.org/albums/alvin-lucier-i-am-sitting-in-a-room/notes
2. **Process.** Iterate one fixed lossy channel. The content fades, and the channel's own signature (its "eigen-image") remains.
3. **Visual behaviour.** A captured form is re-recorded once per bar. It gradually becomes a texture of spots or stripes with one characteristic spacing: the engine's "room".
4. **Engine sketch.**
   - Per bar, one quarter-resolution pass: Gaussian blur (σ 1.5 texels), then unsharp mask (amount about 1.6), then clamp, then mild quantisation.
   - Iterated blur-and-sharpen tends toward Turing-like reaction-diffusion patterns. *This is practitioner knowledge I did not verify with a source in this session: treat it as an inference to test.* That would visually rhyme with the existing **Morphogen** scene.
   - Run **32 generations = 32 bars** as a cued process, as a nod to Lucier. At 70 bpm in 4/4, that is about 110 s.
   - Cost: **S** (one small pass per bar).
5. **Text.** *"We reach for it anyway. / We slow it down. / We replay it."* The replay is itself what dissolves the moment.

### 10. The Basinski dropout: losses that never heal

1. **Finding.** Basinski's 1980s tape loops shed oxide as they were digitised in 2001. Each pass eroded them further, and *The Disintegration Loops* record that decay. [search] https://www.soundonsound.com/techniques/classic-tracks-william-basinski-disintegration-loops · https://crackmagazine.net/article/long-reads/how-william-basinskis-masterpiece-the-disintegration-loops-captured-a-world-crumbling-around-us-in-slow-motion/
2. **Process.** Damage happens at *fixed positions* on the medium and accumulates with each pass. It never moves and never heals.
3. **Visual behaviour.** Holes appear in a recalled image **at the same places every time**, and widen with each recall. This differs from grain or Dust, which are random per frame.
4. **Engine sketch.** Each slot has a fixed noise field N (seeded per slot). The hole mask is `step(N, d)`, with `d += 0.015` per recall. Soften it with a 1-texel feather. Cost: **S**.
5. **Text.** *"And maybe, / every time we remember, / we leave a part of ourselves behind."*

### 11. Replay: compressed, in slices, and in reverse

1. **Findings.**
   - **Reverse replay.** Awake rats pausing at a reward site replay their just-run path in **reverse order** (Foster & Wilson 2006). [search] https://www.nature.com/articles/nature04587 · https://pubmed.ncbi.nlm.nih.gov/16474382/
   - **Compressed replay.** During sharp-wave ripples, sequences that took seconds are replayed in about 50–120 ms (Lee & Wilson 2002). [search] https://elifesciences.org/articles/71850
   - **Human recall is compressed too.** People replay real-life events about **8× faster** than they happened, as "slices" with gaps (Jeunehomme & D'Argembeau). [search] https://www.tandfonline.com/doi/full/10.1080/09658211.2017.1406120 · https://pubmed.ncbi.nlm.nih.gov/33686918/
   - **Boundaries are remembered better.** People split activity into events, and event boundaries are remembered better than the insides of events (Zacks, event segmentation). [search] https://pubmed.ncbi.nlm.nih.gov/22468032/
2. **Process.** Memory samples the past densely at boundaries and sparsely inside events. It plays the samples back fast, discontinuously and sometimes backwards.
3. **Visual behaviour.** A stuttering, fast, gapped playback of the engine's *own* past frames, never footage of the dancers. At the finale it runs backwards through the whole show.
4. **Engine sketch.**
   - **Storage.** A ring atlas: one 4096×4096 R8 texture (16 MB) holds 448 frames at 256×144.
   - **Capture policy.**
     - one frame every 2 bars as a baseline
     - plus 3 frames at every boundary: a scene switch, `crossedBar` together with a `flux` spike, or a `build` release

     That covers the whole show at low density.
   - **Replay modes.** Forward or reverse, at 8× compression with holds on the boundary frames.
   - **Check.** `GL_MAX_TEXTURE_SIZE` on this Iris Xe driver: unknown, test it.
   - Cost: **M** (16 MB of VRAM shared with system RAM; per frame only 1 blit and 1–2 fetches).
5. **Text.** *"We slow it down. / We replay it. / … / But the moment itself never returns. / Only the memory does."*
   - **"Because" (lore, not fact).** Per Songfacts and Classic FM, Lennon wrote the song after asking Ono to play the *Moonlight Sonata*'s chords backwards. [search] https://www.songfacts.com/facts/the-beatles/because · https://www.classicfm.com/composers/beethoven/guides/beatles-because-beethoven-moonlight-sonata/
   - Snopes presents it as a question to check. [search] https://www.snopes.com/fact-check/because/ The page was not opened, so its verdict is unknown.
   - The *reverse replay at rest after a run* is verified science. Its link to the song rests on that story.

### 12. Pattern completion and separation: the present as a torch

1. **Findings.**
   - CA3 performs **pattern completion**: it retrieves a whole stored pattern from a partial or degraded cue. The dentate gyrus performs **pattern separation**: it keeps similar inputs distinct (Yassa & Stark 2011). [search] https://www.psychologie.uzh.ch/dam/jcr:b2297a22-3f83-4262-9e60-dad7a5d4db21/Yassa%20und%20Stark%20-%202011%20-%20TiNS.pdf
   - Hopfield's network "correctly yields an entire memory from any subpart of sufficient size". [search] https://www.pnas.org/doi/10.1073/pnas.79.8.2554
2. **Process.** A fragment of the present calls up the whole of a stored pattern, and completes it toward the *stored* version rather than the present one.
3. **Visual behaviour.**
   - **Completion.** The live scene's light acts as a **torch**. Wherever the live forms are lit, the stored memory shows through inside them.
   - **Inversion.** Later the roles swap: the memory masks the live scene.
4. **Engine sketch.**
   - Completion: `out = scene + mem·smoothstep(0.05, 0.3, luma(scene))·k`.
   - Inversion: `out = scene·memAlpha`.
   - Separation: when two slots overlap, offset them by ±2% of the screen and give them distinct palette positions.
   - Cost: **S**.
5. **Text.** Completion serves *"settling in places / we never expect"*. The inversion is movement 8: *"we no longer carry the memory. / The memory carries us."*

### 13. Constructive simulation: fragments recombined into new images

1. **Finding.** The constructive episodic simulation hypothesis (Schacter & Addis 2007) holds that episodic memory recombines fragments of past episodes to simulate new events. The same recombination causes memory errors. [search] https://philpapers.org/rec/SCHOTC-7
2. **Process.** New scenes are assembled from pieces of stored ones.
3. **Visual behaviour.** Just after the pivot, the screen becomes a mosaic of soft-edged cells. Each cell shows a different memory slot, and the cells are re-dealt each bar. The result is images that never happened, built from moments that did.
4. **Engine sketch.** A Voronoi field of 12–40 cells (a 9-tap search). Each cell picks a slot by `hash(cell, bar)`, with feathered borders. Cost: **S**.
5. **Text.** *"It no longer recalls. / It creates."* Use it right after idea 5.

### 14. Memory colour and the misinformation effect: the present recolours the past

1. **Findings.**
   - **Memory colour.** Knowing an object's typical colour shifts how its colour looks. Grey bananas looked slightly yellow, so observers set them bluish to see them as grey (Hansen et al. 2006). [search] https://www.nature.com/articles/nn1794
   - **Misinformation.** Leading verbs changed memories. "Smashed" produced 32% false reports of broken glass, against 12% in the control (Loftus & Palmer 1974). Loftus (2005) reviews 30 years of the effect. [search] https://www.simplypsychology.org/loftus-palmer.html · https://learnmem.cshlp.org/content/12/4/361.full
   - **Labels.** Captions shifted how drawings were reproduced: 73–74% resembled the caption, against 45% with no label (Carmichael, Hogan & Walter 1932). [search] https://philpapers.org/rec/CARAES-2
2. **Process.** Later information, whether a colour prior, a word or a label, is folded into the recalled trace.
3. **Visual behaviour.**
   - **Palette drift.** Memories are stored as **luminance only** and recoloured at display. As the show's palette drifts over the evening, *old images take on today's colours*.
   - **Names as attractors.** A word from the Text source can become the attractor field of idea 3. A memory "named" in one section bends toward the glyph's shape in the next.
   - **Warning.** Carmichael's famous stimulus was "eye-glasses". Any label or attractor must avoid two round holes.
4. **Engine sketch.**
   - Slots are R8 luminance.
   - Display colour is `mix(capturePalette, currentPalette, clamp(0.25·n, 0, 1))` through the existing 3-colour gradient map. The capture palette is stored as 3 colours per slot.
   - The label attractor reuses the text render as P.
   - Cost: **S**. This carries the brief's "palette drift" seed.
5. **Text.** *"The colors slowly change."* And *"We give it names."*

### 15. Iconic memory and afterimages: the moment that is already leaving

1. **Findings.**
   - Sperling (1960) found that brief displays leave a high-capacity visual trace lasting about **0.3 s** (iconic memory). Partial report recovered about 75% of a cued row. [search] https://sites.socsci.uci.edu/~whipl/staff/sperling/PDFs/Sperling_PsychMonogr_1960.pdf · https://www.simplypsychology.org/iconic-memory.html
   - After adaptation, a complementary **negative afterimage** decays roughly exponentially over seconds. [search] https://www.illusionsindex.org/i/negative-afterimages
2. **Process.** What was just seen persists briefly as an image, then as its inverse.
3. **Visual behaviour.**
   - **Iconic hold.** After a cut or a HIT, the previous frame holds for 300 ms, then drops.
   - **Negative afterimage.** On a mostly-black frame a true negative would be a bright field, which is wrong for this show. So the afterimage is **subtracted from the lifted film base**: a dark ghost of the vanished shape sits in the Blacks for 2–4 s.
4. **Engine sketch.**
   - One 480×270 R8 grab at the event.
   - In the final shader, `blacksLift *= 1 − 0.8·ghost·exp(−t/1.2 s)`.
   - The iconic hold is a 0.3 s freeze of the trail buffer.
   - It adds no flashes, so it stays inside the 3-per-second cap. Cost: **S**.
5. **Text.** *"Movement disappears before we can hold it."* And *"Every moment is already leaving / while we are still living inside it."*

### 16. Perceptual memory that leans forward or back (lower priority)

**Serial dependence.** What we see now is pulled toward what we saw over the last few seconds (Fischer & Whitney 2014). [search] https://www.nature.com/articles/nn3689
- The existing Trails is already a crude "continuity field". A spatially tuned version adds little.

**Representational momentum.** The remembered final position of a moving target is displaced **forward**, in the direction of motion (Freyd & Finke 1984). [search] https://www.researchgate.net/publication/232516958_Representational_momentum
- A "leading ghost" would need motion estimation. Cost: **M**, low value. Skip unless a scene exposes velocity.
- The one worthwhile use is dramaturgical. When the music stops, freeze the image slightly *ahead* of where it was: *"Life keeps moving."*

### 17. Visual long-term memory has vast capacity (a counterweight)

**Finding.** People who viewed 2,500 objects over 5.5 hours could later tell them apart even from the same object in a different state (Brady et al. 2008). [search] https://www.pnas.org/doi/abs/10.1073/pnas.0803390105

**Use.** Not every memory should decay. One or two slots stay **pristine all show**. The contrast between them and the rewritten slots makes the rewriting legible. This matches the text's opening claim that memories do not fade.

---

## Ranking: dramaturgical value × buildability

Both are scored from 1 to 5. Buildability counts the engine work, the risk to the 60 fps budget, and the fit with one operator.

| Rank | Idea | Value | Build | Score | GPU | Serves |
|---|---|---|---|---|---|---|
| 1 | 5. Boundary extension on recall | 5 | 5 | 25 | S | pivot, "It creates" |
| 2 | 1+2. Reconsolidation write-back, prediction-error gated | 5 | 4 | 20 | S | frame / rewrite |
| 3 | 3. Serial-reproduction attractor (everything converges to the mark) | 5 | 4 | 20 | S | ending, "Only a mark" |
| 4 | 4. Gist / verbatim / outline split | 5 | 4 | 20 | S | "keeps the outline" |
| 5 | 6. Clarity paradox (recall hardens) | 4 | 5 | 20 | S | "oldest … clearest" |
| 6 | 7. Involuntary, cue-matched resurfacing | 5 | 3 | 15 | S | "A voice. A scent. A familiar light." |
| 7 | 11. Compressed / reverse replay atlas | 5 | 3 | 15 | M | replay; "Because" |
| 8 | 8. Power-law dormancy + savings | 3 | 5 | 15 | S | "They wait" |
| 9 | 14. Luminance-only memories recoloured by today's palette | 3 | 5 | 15 | S | "colors slowly change" |
| 10 | 10. Basinski fixed dropouts | 3 | 5 | 15 | S | "leave a part of ourselves" |
| 11 | 12. Torch completion / inversion | 4 | 4 | 16* | S | inversion |
| 12 | 13. Voronoi recombination | 4 | 4 | 16* | S | "It creates" |
| 13 | 9. 32-generation Lucier re-record | 4 | 3 | 12 | S | "We replay it" |
| 14 | 15. Iconic hold + afterimage in the blacks | 3 | 4 | 12 | S | "already leaving" |
| 15 | 17. One pristine slot (control) | 2 | 5 | 10 | S | "They don't [fade]" |
| 16 | 16. Representational momentum | 2 | 2 | 4 | M | "Life keeps moving" |

\*Ideas 12 and 13 score 16, above ideas 6–10. I rank them lower because they are **modes of display** that depend on ideas 1–5 existing first.

**All of the top five are one feature:** a `MemoryBank` with a write-back pass. They are its parameters, not separate builds. Estimated total cost:
- one quarter-resolution pass, only on recall
- 3–6 extra fetches in the final shader
- about 1–10 MB of VRAM, or about 26 MB with the replay atlas

That fits the Iris Xe comfortably *(inference; measure with Intel GPA as per RESEARCH-REPORT §B7)*.

---

## What this means for our show

1. **Build one feature, not six.** A `MemoryBank` with 8 slots of quarter-resolution R8 plus gist/edge textures, a per-slot recall count, and one write-back shader. The seeds "show memory", "edge-only memory" and "palette drift" in brief §7.6 all become modes of it.
2. **Memory must survive scene switches.** `PresetManager.cpp:234` clears Trails on every switch. The bank must be exempt, and it must be composited **before crush/palette** in LookPass's final shader, so grain and dust fall on memories too.
3. **Capture at the opening, and at boundaries.** Plan 6–8 cued captures, weighted toward the first third of the show. That front-weighting is the show's "reminiscence bump" (idea 6). The finale should mostly recall early images.
4. **Recall is always a copy of the last recall.** Each cued recall runs the write-back once. So recall #1 in section 3 and recall #4 in the finale differ visibly and deterministically: blur, then hardening, then drift to the attractor.
5. **Make rewriting rare and correlated.** Gate the write-back strength by the prediction error (the difference between the memory and the present, plus `flux`). Recalls over similar material barely change. Recalls over contrasting material change a lot.
6. **The pivot line is boundary extension.** At *"It no longer recalls. It creates."*, the recalled frame shrinks to about 0.85 of the screen. The border becomes visible, and generated continuation grows outside it at once, not faded in. Follow it with the Voronoi recombination for 16–32 bars.
7. **Keep the outline, lose the story.** Detail decays over about 4 bars, masses over about 90 s, and the edges last about 5 minutes, softening slowly. The dancers perform in front of outlines, not full images. That also helps the luminance budget.
8. **The oldest memory is the sharpest.** By the finale the most-recalled slot is a hard two-tone, close to the One Bit look: crisp and confident, and false.
9. **Choose one attractor for the whole show, and make it the last image.** Every memory drifts toward one static prior field, for example a single thin vertical seam of light or a low horizon (never two dots). By the end of "Because", all slots have converged onto it: the "mark" of the text's last lines.
10. **Let memories wait, not vanish.** Use a power-law decay to a floor of about 3–6% luminance, so earlier images stay as ghosts in the blacks. Each re-surfacing rises faster (savings).
11. **Involuntary returns only in quiet passages.** Cosine match of the spectrum fingerprint > 0.92, `presence` < 0.3, at most one per section, cooldown ≥ 90 s, on an arm/disarm toggle. This suits the sparse/ambient half of the score. Add a pad for a manual "familiar light".
12. **Colour from the present.** Store luminance only. Recoloured memories take on the palette of the moment they return, over a whole-show drift, for example Bone → Nitrate → Ash. That makes *"The colors slowly change"* happen without a separate feature.
13. **The "Because" finale runs a reverse replay.** Play the replay atlas backwards over the song, at about 1.5 frames/s, from the last section to the first. It lands on the opening image, rewritten into the mark. The backwards-chords story behind "Because" is **lore** (label it so in CONCEPT.md). Reverse replay at rest after a run is **verified** (Foster & Wilson 2006). Never quote the lyrics.
14. **Stay recoverable.** Write every captured slot and its recall count to disk (PNG plus JSON) at capture. Add "load show memory" so any section can be rehearsed from a known state without running the whole show. Use deterministic seeds per (slot, recall).
15. **Luminance and safety.** Cap memory composites at about 30% of the scene's peak, blended with screen or max like Trails. Keep them in the upper and outer thirds when dancers need light. The afterimage is a *subtraction* from the lifted blacks, never a flash, so the 3-per-second limiter is untouched.

---

## Sources

WebFetch attempts: pubmed.ncbi.nlm.nih.gov, pmc.ncbi.nlm.nih.gov and www.jneurosci.org were all blocked by the egress proxy. So every source below is **[search]**: seen only in WebSearch results. None was opened.

**Reconsolidation and retrieval**
- [search] Nader, Schafe & LeDoux 2000, Nature: https://www.nature.com/articles/35021052
- [search] same, PubMed: https://pubmed.ncbi.nlm.nih.gov/10963596/
- [search] Bridge & Paller 2012, J Neurosci: https://www.jneurosci.org/content/32/35/12144
- [search] Bridge & Paller, PubMed: https://pubmed.ncbi.nlm.nih.gov/22933797/
- [search] PsyPost on Bridge & Paller: https://www.psypost.org/each-time-you-recall-an-event-your-brain-distorts-it/
- [search] Sevenster, Beckers & Kindt 2013, PubMed: https://pubmed.ncbi.nlm.nih.gov/23413355/
- [search] same, Science summary: https://science.sciencemag.org/content/339/6121/830.editor-summary

**Reconstruction and serial reproduction**
- [search] Serial reproduction of an urban myth (revisiting Bartlett): https://www.tandfonline.com/doi/full/10.1080/09658211.2022.2059514
- [search] Bartlett revisited, repeated vs serial reproduction: https://www.sciencedirect.com/science/article/abs/pii/S2211368114000485
- [search] Bergman & Roediger 1999: https://link.springer.com/article/10.3758/BF03201224
- [search] Langlois et al. 2021, PNAS: https://www.pnas.org/doi/10.1073/pnas.2012938118
- [search] Langlois et al., PDF: https://cocosci.princeton.edu/papers/langloisserial.pdf
- [search] Huttenlocher, Hedges & Duncan 1991 (category bias in location memory; supports idea 3): https://pubmed.ncbi.nlm.nih.gov/1891523/

**Gist, verbatim and transformation**
- [search] Fuzzy-trace theory review: https://pubmed.ncbi.nlm.nih.gov/11605365/
- [search] Brainerd & Reyna 2002: https://journals.sagepub.com/doi/10.1111/1467-8721.00192
- [search] Winocur & Moscovitch 2011: https://pubmed.ncbi.nlm.nih.gov/21729403/

**Boundary extension and perspective**
- [search] Intraub & Richardson 1989: https://pubmed.ncbi.nlm.nih.gov/2522508/
- [search] Boundary extension, Scholarpedia: http://www.scholarpedia.org/article/Boundary_extension
- [search] Intraub & Dickinson 2008: https://www.ncbi.nlm.nih.gov/pubmed/19000211
- [search] Nigro & Neisser 1983: https://www.sciencedirect.com/science/article/abs/pii/0010028583900166

**Confidence, the bump and involuntary memory**
- [search] Talarico & Rubin 2003: https://journals.sagepub.com/doi/abs/10.1111/1467-9280.02453
- [search] Talarico & Rubin 2003, PDF: https://sites.lafayette.edu/talaricj/files/2009/09/TalaricoRubin2003.pdf
- [search] Reminiscence bump (citing Rubin, Wetzler & Nebes 1986): https://pmc.ncbi.nlm.nih.gov/articles/PMC6841349/
- [search] Glück & Bluck 2007: https://lifestorylab.psych.ufl.edu/wp-content/uploads/sites/84/Bump.Glueck.Bluck_.2007.pdf
- [search] Berntsen, BPS: https://www.bps.org.uk/psychologist/involuntary-autobiographical-memories
- [search] Episodic nature of involuntary memories: https://pubmed.ncbi.nlm.nih.gov/15552356/
- [search] Involuntary memories and spontaneous thought: https://pmc.ncbi.nlm.nih.gov/articles/PMC7741080/
- [search] Chu & Downes 2000: https://pubmed.ncbi.nlm.nih.gov/10668001/
- [search] Swann's Way overture: https://www.sparknotes.com/lit/swannsway/section1/
- [search] Godden & Baddeley replication (Murre 2021): https://royalsocietypublishing.org/rsos/article/8/11/200724/95735/The-Godden-and-Baddeley-1975-experiment-on-context

**Forgetting**
- [search] Murre & Dros 2015: https://journals.plos.org/plosone/article?id=10.1371%2Fjournal.pone.0120644
- [search] Wixted & Ebbesen 1991: https://journals.sagepub.com/doi/10.1111/j.1467-9280.1991.tb00175.x

**Replay and events**
- [search] Foster & Wilson 2006: https://www.nature.com/articles/nature04587
- [search] same, PubMed: https://pubmed.ncbi.nlm.nih.gov/16474382/
- [search] SWR replay model (cites Lee & Wilson 2002): https://elifesciences.org/articles/71850
- [search] Jeunehomme & D'Argembeau, temporal compression: https://www.tandfonline.com/doi/full/10.1080/09658211.2017.1406120
- [search] Slices of the past: https://pubmed.ncbi.nlm.nih.gov/33686918/
- [search] Event segmentation: https://pubmed.ncbi.nlm.nih.gov/22468032/

**Completion and simulation**
- [search] Yassa & Stark 2011, PDF: https://www.psychologie.uzh.ch/dam/jcr:b2297a22-3f83-4262-9e60-dad7a5d4db21/Yassa%20und%20Stark%20-%202011%20-%20TiNS.pdf
- [search] Hopfield 1982: https://www.pnas.org/doi/10.1073/pnas.79.8.2554
- [search] Schacter & Addis 2007: https://philpapers.org/rec/SCHOTC-7

**Colour, misinformation and labels**
- [search] Hansen et al. 2006: https://www.nature.com/articles/nn1794
- [search] Loftus & Palmer 1974: https://www.simplypsychology.org/loftus-palmer.html
- [search] Loftus 2005: https://learnmem.cshlp.org/content/12/4/361.full
- [search] Carmichael, Hogan & Walter 1932: https://philpapers.org/rec/CARAES-2

**Perception and persistence**
- [search] Sperling 1960, PDF: https://sites.socsci.uci.edu/~whipl/staff/sperling/PDFs/Sperling_PsychMonogr_1960.pdf
- [search] Iconic memory: https://www.simplypsychology.org/iconic-memory.html
- [search] Negative afterimages: https://www.illusionsindex.org/i/negative-afterimages
- [search] Fischer & Whitney 2014: https://www.nature.com/articles/nn3689
- [search] Representational momentum: https://www.researchgate.net/publication/232516958_Representational_momentum
- [search] Brady et al. 2008: https://www.pnas.org/doi/abs/10.1073/pnas.0803390105

**Media analogues**
- [search] Generation loss: https://en.wikipedia.org/wiki/Generation_loss
- [search] JPEG quality loss: https://uploadcare.com/blog/jpeg-quality-loss/
- [search] MoMA on Lucier: https://www.moma.org/explore/inside_out/2015/01/20/collecting-alvin-luciers-i-am-sitting-in-a-room/
- [search] Lucier, DRAM notes: https://www.dramonline.org/albums/alvin-lucier-i-am-sitting-in-a-room/notes
- [search] Basinski, Sound On Sound: https://www.soundonsound.com/techniques/classic-tracks-william-basinski-disintegration-loops
- [search] Basinski, Crack: https://crackmagazine.net/article/long-reads/how-william-basinskis-masterpiece-the-disintegration-loops-captured-a-world-crumbling-around-us-in-slow-motion/

**"Because" (lore)**
- [search] Songfacts: https://www.songfacts.com/facts/the-beatles/because
- [search] Classic FM: https://www.classicfm.com/composers/beethoven/guides/beatles-because-beethoven-moonlight-sonata/
- [search] Snopes (verdict unknown, not opened): https://www.snopes.com/fact-check/because/
