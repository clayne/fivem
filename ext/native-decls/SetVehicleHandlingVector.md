---
ns: CFX
apiset: client
game: gta5
---
## SET_VEHICLE_HANDLING_VECTOR

```c
void SET_VEHICLE_HANDLING_VECTOR(Vehicle vehicle, char* class_, char* fieldName, Vector3 value);
```

Sets a handling override for a specific vehicle. Certain handling flags can only be set globally using `SET_HANDLING_VECTOR`, this might require some experimentation.

## Parameters
* **vehicle**: The vehicle to set data for.
* **class_**: The handling class to set. Only "CHandlingData" is supported at this time.
* **fieldName**: The field name to set. These match the keys in `handling.meta`.
* **value**: The Vector3 value to set.

