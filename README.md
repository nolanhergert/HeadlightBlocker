# HeadlightBlocker
Note! Definitely AI assisted

I'm prototyping low-cost eyewear that selectively dims intense light — sunlight, reflected glare, oncoming headlights — while leaving the rest of the scene visible.

Regular sunglasses only have one trick: darken everything, all the time. That works, but it also means you're seeing the whole world through a dimmer switch just to deal with one bright patch of it. I wanted to know if I could instead darken only the area around the bright source and leave everything else alone — dim just the headlight, not the whole road.

This repo is where I've been working through the optical, electronic, firmware, and business questions behind that idea. It's an active exploration, not a finished product, and nothing here should be treated as a driving-safety or medical device.

## What I'm actually trying to answer

Three questions keep coming up:

1. Can cheap, low-power hardware actually pull off useful selective dimming?
2. Are there enough people who feel this problem strongly enough to pay for a fix?
3. If the answer to both is yes, why doesn't this already exist?

I don't have final answers, but I've been chasing all three at once: tens of customer discovery interviews to understand how people actually experience glare and whether they'd pay to fix it, conversations with people who've founded companies in or around this space to get an honest read on where the business risk really lives, and simulations, bench tests, and physical prototypes to see what's technically achievable. None of it tells me whether this should become a company — it just cuts the uncertainty down one conversation and one experiment at a time, instead of assuming "technically interesting" means "good business."

My current best guess at early adopters is people with daytime light sensitivity and drivers badly bothered by headlight glare at night; pilots, cyclists, athletes, and professional drivers seem like plausible extensions, but those are hypotheses, not validated customers.

## Current technical directions

### Designing a pixel pattern that disappears

Hold any LCD close to your eye and the pixel grid itself becomes part of what you see: the gaps between pixels and the traces connecting them blur together with whatever light is passing through, closer to halftone dithering in a newspaper photo than to clean, uniform dimming. Getting that pattern to disappear, rather than draw attention to itself, turns out to be its own small design problem: what pixel shape gives the best black-to-transparent ratio at the edges, how do you hide or cover the traces, what ITO layer thickness avoids introducing its own refractive glare, and so on — a lot of interacting choices that are slow and expensive to explore by ordering LCD samples one at a time.

So I built a simulator to try out pixel shapes and spacings before committing to an actual LCD order.

Try it here: [LCD pixel-pattern blur simulator](https://nolanhergert.github.io/HeadlightBlocker/pixel_pattern_blur_simulator.html).

### Getting high contrast out of cheap passive-matrix LCDs

This is a separate problem from the one above, and it's about electronics, not optics. Passive-matrix LCDs are cheap and can be made at high resolution, but they're driven by multiplexing: each row only gets a small slice of the total drive time, and that low duty cycle caps how dark a pixel can actually get. That's why passive-matrix displays usually look washed out compared to active-matrix (TFT) screens — and why TFT is normally the fallback once you need real contrast, at a much higher cost.

I've worked out a way to drive multiple rows in parallel using off-the-shelf chip-on-glass driver chips instead of custom silicon, which raises the effective duty cycle and gets much closer to TFT-like contrast on a passive-matrix panel. The goal is high resolution and high contrast without paying for a custom TFT display or its minimum order quantities.

### Low-power sensing and tracking

The system also needs to find bright, localized sources, figure out roughly where they are, and keep tracking them as the scene changes — all without running a full always-on vision pipeline, since that would blow through the power budget almost immediately. Staying at ultra-low power shapes almost every decision here: sensor choice, sampling rate, how much you can afford to compute versus just measure directly.

### Low-level embedded control

I've been teaching myself Rust while working on the low-level control paths on a CH32-series microcontroller. Incoming frame data comes in via DMA so the CPU isn't stuck babysitting the transfer, then gets warped to line up with the display's geometry and inverted into a blocking mask, fast enough inside the CPU to keep up with a moving light source. That warp is specific to each user, since it depends on exactly how the display sits relative to their eyes, so it needs its own calibration step. Other areas I've been digging into: how the watchdog timer actually behaves under real firmware conditions instead of just the datasheet version, and how to squeeze out power savings without adding latency the system can't afford. All of this ties back to the same constraint: the system has to react to a changing light source quickly, on a tiny power and memory budget.

The Rust code itself isn't published — the camera chip's register-level details are covered by an NDA, and the rest is scattered across a pile of exploratory prototypes I wrote while learning the chip and the language at the same time, not exactly something I'd call presentable.

## Earlier work and demonstrations

The demos below are from DynEye, an earlier project and startup founded by Chris Mullin that took on a similar idea before I got involved. Good historical context, and worth a look at the broader concept in action:

DynEye demo clips:
https://github.com/nolanhergert/HeadlightBlocker/assets/377502/8a296c67-bb7e-49ee-8bd9-374424baa6de

https://github.com/nolanhergert/HeadlightBlocker/assets/377502/1ff41848-ae48-455d-8731-473bc94387cb

[DynEye archive](https://web.archive.org/web/20240226041115/http://www.dyneye.com/)

Press coverage: [Popular Science: Glare Killer](https://www.popsci.com/diy/article/2011-05/2011-invention-awards-glare-killer/), [Inside Science: LCD sunglasses](https://www.insidescience.org/video/lcd-sunglasses-block-glare-moving-pixels)

A CBS News segment on nighttime headlight glare (not about DynEye specifically, but a good overview of the underlying problem):

[![CBS News segment on nighttime headlight glare](http://img.youtube.com/vi/w0nBlZwUT3s/0.jpg)](https://www.youtube.com/watch?v=w0nBlZwUT3s "CBS News segment on nighttime headlight glare")

## Following along

This is intentionally open-ended, and I'm happy to hear alternative approaches, useful references, or blunt criticism on any topic interesting to you!

For the messier day-to-day notes, experiments, and project history, see the [project wiki](https://github.com/nolanhergert/HeadlightBlocker/wiki). Fair warning: it's scattered rough notes rather than a polished narrative, but it's the most honest record of how this has actually gone.
