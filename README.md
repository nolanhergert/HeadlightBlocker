# HeadlightBlocker

Survey!: https://docs.google.com/forms/d/e/1FAIpQLSdY2P8LRLevYkaR5bCc3iHimnQf3yeSkIpHeLKnRqFuZlO9xA/viewform?usp=header

I'm prototyping low-cost eyewear that selectively dims bright light sources—sunlight, reflected glare, oncoming headlights—without unnecessarily darkening the rest of your field of view.

Ordinary sunglasses solve glare by reducing all incoming light. This works well during the day, but it is a fairly blunt instrument: if one small part of the scene is uncomfortably bright, you make everything darker to compensate. The basic idea behind this project is to selectively block the bright part instead.

This repository documents my attempt to figure out whether that idea is technically practical, economically useful, or neither.

## What I'm trying to learn

There are three questions that matter:

1. Can inexpensive, low-power hardware provide useful selective dimming?
2. Do enough people care about glare enough to pay for a solution?
3. If both are true, why hasn't someone already built a successful product?

The third question is particularly important. "Nobody has done this yet" is sometimes evidence of an opportunity. It is also frequently evidence that you have not yet discovered the problem that killed everyone else's attempt.

I've been attacking these questions in parallel. I've conducted dozens of customer-discovery interviews about how people experience glare and what, if anything, they would pay to do about it. I've also spoken with current and former founders in and around the space to understand where previous approaches ran into trouble.

On the technical side, I've used simulations, bench experiments, and targeted physical prototypes to reduce the cost of answering individual questions. Building the entire product repeatedly would be slow and expensive, and usually isn't necessary to learn whether one particular assumption is wrong.

None of this establishes that the idea should become a company. The goal is to reduce uncertainty one experiment and one conversation at a time, rather than confuse an interesting engineering problem with a viable business.

My current hypothesis is that the earliest users would be people with significant daytime light sensitivity and people particularly bothered by headlight glare at night. Pilots, cyclists, athletes, and professional drivers are plausible adjacent markets. I have substantially less evidence for those.

As it turns out, many of the hardest questions aren't technical:

* **Economics:** Consumer hardware is a difficult business. Revenue is mostly transactional rather than recurring, consumers are price-sensitive, and engineering, tooling, inventory, and support all cost substantially more than shipping another copy of a software product.
* **Form factor and styling:** One size—and one style—doesn't fit all in eyewear. Should this be a standalone pair of glasses, a clip-on, or a module that eyewear manufacturers can integrate into their own frames?
* **Liability:** People may use the product in safety-critical situations. Even a product that works as designed could be involved in an accident and, consequently, a lawsuit.
* **Marketing and distribution:** Eyewear is something people often want to try before buying. Giving customers a convenient way to evaluate the usefulness, appearance, and ergonomics of an unfamiliar product requires distribution—and everyone involved in that distribution needs to get paid.

## Current technical work

### Making an LCD pixel pattern disappear

Put an LCD close enough to your eye and its pixel structure becomes part of the image.

The gaps between pixels and the conductive traces connecting them interact with incoming light, producing something closer to the visual effect of halftone dithering than uniform dimming. For eyewear, this creates an unusual optimization problem: you don't merely want a high-resolution display. You want a display whose structure is as unobjectionable as possible when viewed from a few centimeters away.

That raises a collection of related questions. What pixel geometry produces the best transition between blocked and transparent regions? How should the traces be routed or hidden? How much does the sheet resistance and thickness of the ITO matter? Can the electrode geometry itself create noticeable diffraction or refractive glare?

Custom LCD samples are relatively slow and expensive iterations, so I built a simulator to explore pixel geometries and spacing before ordering glass.

Try it here: [LCD pixel-pattern blur simulator](https://nolanhergert.github.io/HeadlightBlocker/pixel_pattern_blur_simulator.html).

### Getting more contrast from passive-matrix LCDs

The next problem is electrical rather than optical.

Passive-matrix LCDs are inexpensive and can support fairly high resolutions, but multiplexing imposes an important limitation. Each row receives only part of the available drive time. As the multiplex ratio increases, the distinction between the voltage seen by an "on" pixel and an "off" pixel gets progressively worse.

The usual solution when you need substantially better contrast is an active-matrix TFT display, which costs at least 1 million USD and large minimum order quantities (MOQ).

I'm exploring an intermediate approach: driving multiple groups of rows in parallel using commodity chip-on-glass shift-register drivers. This reduces the effective multiplex ratio without requiring custom driver silicon.

### Finding bright objects without wasting power

The glasses need to identify small, bright light sources, estimate where they are, and continue tracking them as either the wearer or the source move and change shape.

A conventional always-on computer-vision pipeline would make this relatively straightforward. It would also consume far too much power.

That constraint changes the architecture. Sensor resolution, field of view, sampling rate, motion detection, wake-up behavior, and the division between measurement and computation all matter. A recurring question is whether a small amount of specialized sensing can avoid keeping a comparatively power-hungry camera and processor running continuously.

The goal is not sophisticated scene understanding. The glasses don't need to know that something is a Ford F-150 with its high beams on. They need to know that there is a very bright object at approximately `(x, y)`, and then keep the blocking region aligned with it.

### Low-level embedded control

I'm also using the project as an excuse to learn Rust on a small CH32-series microcontroller.

Camera data arrives through DMA rather than requiring the CPU to service each transfer. The resulting image is geometrically warped into the display's coordinate system and converted into a blocking mask. This has to happen quickly enough that the blocked region continues to track a moving source without an objectionable delay.

The geometric transform is user-specific. The camera, LCD, eye, and scene all occupy different coordinate systems, and small differences in how the glasses sit on someone's face change the mapping. That implies some form of per-user calibration, and there are still unknowns here, such as whether eye tracking is necessary to detect pupil motion and dilation. Ideally this would not be necessary, as it greatly increases the cost and power requirements.

I've also spent time on less glamorous problems that become important in a constrained embedded system: verifying how watchdog behavior differs from the simple mental model suggested by a datasheet, deciding which peripherals can be shut down and when, and reducing idle power without adding unacceptable wake-up latency.

The common constraint is simple: react quickly to changing light sources using very little energy, memory, and compute.

The Rust source is not currently published. Some register-level details for the camera are covered by an NDA, and much of the remaining code consists of exploratory prototypes written while I was learning both the hardware and the language. Publishing that code in its current state would probably create more confusion than value.

## Previous work

I am not the first person to have this idea.

DynEye, a startup founded by Chris Mullin, previously developed eyewear based on a similar concept. I was not involved with DynEye, but its work is useful context for both the technical idea and the history of attempts to commercialize it.

DynEye demo clips:

https://github.com/nolanhergert/HeadlightBlocker/assets/377502/8a296c67-bb7e-49ee-8bd9-374424baa6de

https://github.com/nolanhergert/HeadlightBlocker/assets/377502/1ff41848-ae48-455d-8731-473bc94387cb

[DynEye archive](https://web.archive.org/web/20240226041115/http://www.dyneye.com/)

Press coverage: [Popular Science: Glare Killer](https://www.popsci.com/diy/article/2011-05/2011-invention-awards-glare-killer/) and [Inside Science: LCD sunglasses](https://www.insidescience.org/video/lcd-sunglasses-block-glare-moving-pixels).

For background on nighttime headlight glare more generally, CBS News also covered the problem here:

[![CBS News segment on nighttime headlight glare](http://img.youtube.com/vi/w0nBlZwUT3s/0.jpg)](https://www.youtube.com/watch?v=w0nBlZwUT3s "CBS News segment on nighttime headlight glare")

## Following the project

This project is deliberately open-ended. Alternative technical approaches, relevant prior art, useful references, and criticism are all welcome.

For day-to-day experiments and project history, see the [project wiki](https://github.com/nolanhergert/HeadlightBlocker/wiki).

The wiki is not a polished narrative. It is closer to a lab notebook: incomplete experiments, abandoned ideas, intermediate conclusions, and the occasional realization that something I thought would work did not. That's also a more accurate record of how the project has actually developed.
