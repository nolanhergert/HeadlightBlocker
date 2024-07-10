# HeadlightBlocker
Are you like tons of people annoyed with the sun or headlights hurting your eyes? Nice video: https://www.youtube.com/watch?v=w0nBlZwUT3s

There's an *actual* solution for this, here are some demo videos from a **previous startup** that went under, DynEye:

https://github.com/nolanhergert/HeadlightBlocker/assets/377502/8a296c67-bb7e-49ee-8bd9-374424baa6de

https://github.com/nolanhergert/HeadlightBlocker/assets/377502/1ff41848-ae48-455d-8731-473bc94387cb

https://www.popsci.com/diy/article/2011-05/2011-invention-awards-glare-killer/, https://www.insidescience.org/video/lcd-sunglasses-block-glare-moving-pixels
## Features
  * Will dim all bright lights anywhere in your vision, including side view mirrors, reflected glare off the car / road (when raining at night). Sunglasses don't cut it because they darken the entire field of view evenly, which just makes your eyes dilate and then you're back to what you had before!
    
## Justification
  * This shouldn't be a thing: "If oncoming traffic lights are too bright or blinding, focus eyes toward the edge of the road."
     * It's painful for your rods/cones to over expose. Since the light source is small, usually your iris won't dilate much either.
     * It takes time for your eyes to dilate again, especially if you are older.
  * "Can’t comment on other parts of the country, but in California it’s pretty much mandatory to wear sunglasses during sunny days. Which is most of them. The concrete is white and quite blinding in the sunlight. I keep spares in the car just incase, simply can’t drive without them."
  * NHSTA dataset (what does it say??): [https://www.nhtsa.gov/file-downloads?p=nhtsa/downloads/CRSS/2021/](https://www.nhtsa.gov/file-downloads?p=nhtsa/downloads/CRSS/)

## Target Customers
  * All-day drivers/motorcycle/bicyclists/pilots/police/trucks either going into the morning/evening sun and can't use a sun visor but still need to see details
  * Nighttime driving in the countryside, there are occasional glaring headlights and no streetlights to keep your pupils constricted. Causes pain and potentially very dangerous whiteout for multiple seconds. Try it yourself at nighttime / a dark room with your phone LED pointed at your eyes!
    * Especially a problem in India, where drivers use high beams a lot and there are potholes, people crossing, etc. However budget and quality requirements are a lot less than USA.
    * Problem is also worse with older drivers (pupils don't re-dilate quickly) and those with astigmatism and other visual problems (glare "halo" is even wider and more distracting)
    * Tall trucks/SUVs with high headlights still need to see the road too! But cause much suffering for sedan drivers.
    * Drivers of tall vehicles (trucks) still suffer from people using high beams even though they are high up.
  
    

### Stretch Customers
  * Fishing in choppy water. Apparently normal polarized sunglasses don't work? Might be interesting to have a one-filter version that has an LCD for rotating pixels oncoming light so its better cancelled by the filter without making the pixel dark/black.
    * FSTN refresh rate is pretty fast still. https://youtu.be/6h2D-CnLQEc?si=O5FLN6evWkWHE9HI&t=299

See the wiki page: https://github.com/nolanhergert/HeadlightBlocker/wiki

POC OpenCV code (for stepper motor solution, potentially obsolete) is here: https://github.com/nolanhergert/lib/blob/master/python/examples/headlight_blocker.py

Can leverage CH32V003fun to read from camera using DMA? Use this example https://github.com/cnlohr/ch32v003fun/blob/master/examples/dma_gpio/dma_gpio.c but change this line to be INDR? `DMA1_Channel2->PADDR = (uint32_t)&GPIOC->OUTDR;`
