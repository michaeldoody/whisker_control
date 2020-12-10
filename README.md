# Whisker Drawing Mechanism
## Project Obectives

The purpose of this project is to design a device that manufactures robotic whisker sensors by drawing polymer plastic filament from an oven. It must be able to inspect the diameter of the whisker and adjust the draw speed accordingly to match a specific geometric profile. This process must be quick and repeatable. 

The image below shows how the shape of the filament develops over time. The heated length of the filament is brought to the plastic's glass transition temperature. Then the filament is pulled away from the oven by the drawing mechanism to create a taper.
 
 
![Filament Drawing Process](media/whisker_drawing.png)


The artificial whisker geometric profile imitates that of a biological whisker at a 5x scale (see image below). These dimensions drove the design specifications for the drawing mechanism. 


![Whisker Profile](media/whisker_geometry.png)

## Design Specs

Given the dimensions of an artificial whisker, the mechanism must:
- Include a linear actuator that can draw a 250 mm whisker in approx. 2 mins
- Acheive a velocity range of 0.01 mm/s - 10 mm/s
- Include velocity feedback control to regulate the whisker taper
- Have computer vision to inspect the whisker diameter in micrometers
