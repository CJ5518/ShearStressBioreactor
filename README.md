# Shear Stress Bioreactor
Shear Stress Bioreactor project for the University of Idaho 2024 senior capstone project.

Students involved:

* Benjamin Morenas (BE)
* Ishmael Staples (BE)
* Carson Sloan (CS)
* Carson Rueber (CS)
* Chris Bui (CS)
* Guo Zhonghao (EE)

# Value Proposition
Tendon-related injuries impact millions of people and collectively cost millions of dollars in medical expenses every year. Patients are left permanently debilitated in many cases, due to the poor natural healing capacity of tendon tissue. More research into cell mechanobiology is needed to both better understand the causes of such injuries and devise better means of treatment, and experimentation is best performed in an in vitro environment that can be carefully controlled, such as in a bioreactor. Unfortunately, existing bioreactor systems cost tens of thousands of dollars per unit, making them inaccessible to many researchers. Our goal is to design and build an easy-to-use and low-cost shear stress bioreactor that can deliver fluid shear stress to cells in culture without the need for overly expensive equipment. We will produce a fully operable prototype and instructions to replicate it, so it can be created for use by other labs. 

# Repository Overview
The software for the project is developed for the ESP 32 microcontroller with [PlatformIO.](https://platformio.org/) 

The `src/` directory is divided into two directories, one for the hardware focused aspect of the project and one for the web server focused aspect of the project. Together the halves provide an interface to precisely control the bioreactor. 

The `data/` directory contains html, javascript, and css assets for use with the web server. These assets are bundled into a file called `html.hpp` by a [Lua](https://lua.org) script called `buildHtml.lua` which is then used by the web server. For any and all changes to files in the `data/` directory to be included in the built version of the project, buildHtml.lua must be executed to regenerate `html.hpp` so that the web server can find the files.
