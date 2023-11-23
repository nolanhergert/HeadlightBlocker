# HeadlightBlocker
Are you like tons of people annoyed with bright lights in the evening hurting your eyes? There's an *actual* solution for this, here are some demo videos from a previous startup:

View of Dyneye prototype: http://www.youtube.com/watch?v=cSOoYN_iOBY

View of Dyneye blocking sun: https://www.youtube.com/watch?v=PTG62q7hMcY
## Features
  * Will dim all bright lights anywhere in your vision, including side view mirrors, reflected glare off the car / road (when raining at night). Sunglasses don't cut it because they darken the entire field of view evenly, which just makes your eyes dilate and then you're back to what you had before!
    
## Justification
  * This shouldn't be a thing: "If oncoming traffic lights are too bright or blinding, focus eyes toward the edge of the road."
     * It's painful for your eyes to over expose and desperately shrink iris
     * It takes time for your eyes to dilate again
  * NHSTA dataset: [https://www.nhtsa.gov/file-downloads?p=nhtsa/downloads/CRSS/2021/](https://www.nhtsa.gov/file-downloads?p=nhtsa/downloads/CRSS/)

## Target Customers
  * Drivers/motorcycle/bicyclists/pilots either going into the morning/evening sun and can't use a sun visor but still need to see details
  * Same group, but nighttime where there are occasional glaring headlights and no streetlights to keep your pupils dilated. Causes very dangerous whiteout. Try it yourself at nighttime / a dark room with your phone LED pointed at your eyes!
    * Especially a problem in India, where drivers use high beams a lot and there are potholes, people crossing, etc. However budget is a lot less than USA.

### Stretch Customers
  * Fishing in choppy water. Both for polarizing effect of reducing glare but for blocking light? FSTN refresh rate is pretty fast still. https://youtu.be/6h2D-CnLQEc?si=O5FLN6evWkWHE9HI&t=299

See the wiki page: https://github.com/nolanhergert/HeadlightBlocker/wiki

POC OpenCV code (for stepper motor solution, potentially obsolete) is here: https://github.com/nolanhergert/lib/blob/master/python/examples/headlight_blocker.py

Can leverage CH32V003fun to read from camera using DMA? Use this example https://github.com/cnlohr/ch32v003fun/blob/master/examples/dma_gpio/dma_gpio.c but change this line to be INDR? `DMA1_Channel2->PADDR = (uint32_t)&GPIOC->OUTDR;`
