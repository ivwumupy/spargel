# Coordinates

Several units:
- pixel
- point
- (font design) unit
- em

## User Coordinate

The unit for the user coordinate is `point`.
The rough idea is that 1 point should be 1/72 inch.

## Device Coordinate

The unit is `pixel`.
The transformation from user coordinate is determined by the value `pixel per inch`, which is a property of the hardware, and by the equation `1 point = 1/72 inch`.

## Font Design Coordinate to User Coordinate

Every (TrueType/OpenType) font has a value `unit per em`.
When specifying a font size, one pass in a value `point per em`.
