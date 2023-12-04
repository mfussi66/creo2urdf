# creo2urdf
Generate URDF model from CREO Parametric mechanisms.

> [!warning]
>
> ⚠️ LIBRARY UNDER DEVELOPMENT ⚠️

**Since it is under development, we cannot guarantee that the user interface of `creo2urdf` will not implement breaking changes. Be aware of this if you start using the code contained in this repository**

## CREO Parametric versions supported and tested
- `9.0.2`

## Assumptions & limitations

- In order to export the urdf of the robot, the relative CREO mechanism has to conform to [these guidelines](https://github.com/icub-tech-iit/cad-libraries/wiki/Prepare-PTC-Creo-Mechanism-for-URDF).
- The joints are in 0.
- Right now it can handle only **REVOLUTE** and **FIXED** joints.

## Installation from binaries

Download `creo2urdf.zip` from the assets of the [latest release](https://github.com/icub-tech-iit/creo2urdf/releases/latest), and extract it in the folder you like.<br>
Follow the [`Usage`](https://github.com/icub-tech-iit/creo2urdf/tree/master#usage) section instructions for completing the installation. <br>
This archive contains the dll of the plugin and the `text` folder for running it. <br>

## Installation from sources

### Dependencies
Right now the `creo2urdf` plugin needs its dependencies to be compiled and linked **statically**:
- Download [vcpkg](https://github.com/microsoft/vcpkg): `git clone https://github.com/microsoft/vcpkg`
- Bootstrap vcpkg: `.\vcpkg\bootstrap-vcpkg.bat`
- Run `[path to vcpkg]/vcpkg install --triplet x64-windows-static-md eigen3 yaml-cpp rapidcsv libxml2 assimp`
- Install [iDynTree@devel](https://github.com/robotology/idyntree/tree/devel) from sources.
  - It has to be compiled passing these CMake variables `-DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static-md` 

### Build `creo2urdf`

- Export `CREO9_INSTALL_PATH` pointing to the Creo9 installation folder, e.g. `C:\Program Files\PTC\Creo 9.0.2.0\Common Files`
- Pass `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static-md` to the creo2urdf compilation.

For those who use the CMake integration in Visual Studio, the `-DCMAKE_TOOLCHAIN_FILE` option should not be passed to `CMake command arguments`. Instead, the `vcpkg.cmake` file path must be passed in `CMake toolchain file`.


## Usage

- Put in your CREO working directory the `protk.dat` that is automatically generated by CMake in `${PROJECT_BINARY_DIR}` (e.g. `C:\Users\ngenesio\icub-tech-iit\creo2urdf\build\x64-Release`).
Or create it by hand as follow:

```
name creo2urdf
Startup dll
Allow_stop True
Delay_start False
exec_file /path/to/creo2urdf.dll
text_dir /path/to/creo2urdf/src/creo2urdf/text
END
```
- Open CREO Parametric and the first time you will need to accecpt to register the plugin
- Load the mechanism of which you want to export the urdf
- Press the button
 <img src="https://github.com/icub-tech-iit/creo2urdf/assets/19152494/19866e6c-8f52-4010-84ef-db458189b753" width=600 align=center >
- Select a yaml and csv configuration files with a format explained in the following sections.

### YAML Parameter File
The YAML format is used to pass parameters to the plugin to customized the conversion process.
The parameter accepted by the plugin are documented in the following.

##### Severity Parameters
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `warningsAreFatal`     | Boolean     | true | Used for throwing fatal errors in case some steps in the exportation of the urdf are failing. |

##### Naming Parameters
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `robotName`     | String     | model name set in the file PhysicalModelingXMLFile | Used for setting the model name, i.e. the parameter `<robot name="...">` in the `URDF` model file. |
| `rename`        | Map  | {} (Empty Map) | Structure mapping the SimMechanics XML names to the desired URDF names.  |


##### Root Parameters
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:------:|:------------:|:-------------:|
| `root`           | String | First body in the file | Changes the root body of the tree |
| `originXYZ`      |  List  | empty | Changes the position of the root body |
| `originRPY`      |  List  | empty | Changes the orientation of the root body |

##### Algorithm Parameters
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:------:|:-------------:|:-------------:|
| `epsilon`        | Float | 4*(Machine *eps*) | Set a custom value for testing whether a number is close to zero |

##### Frame Parameters
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `linkFrames`       | Array | empty | Structure mapping the link names to the displayName of their desired frame. Unfortunatly in URDF the link frame origin placement is not free, but it is constrained to be placed on the parent joint axis, hence this option is for now reserved to the root link and to links connected to their parent by a fixed joint |
| `exportAllUseradded` |  Boolean | False | If true, export all SimMechanics frame tagged with `USERADDED` in the output URDF as fake links i.e. fake link with zero mass connected to a link with a fixed joint..  |
| `exportedFrames` | Array | empty | Array of `displayName` of UserAdded frames to export. This are exported as fixed URDF frames, i.e. fake link with zero mass connected to a link with a fixed joint. |

###### Link Frames Parameters  (keys of elements of `linkFrames`)
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:-----------:|:-------------:|
| `linkName`       | String |  Mandatory  | name of the link for which we want to set the frame |
| `frameName`      | String | Mandatory  | `displayName` of the frame that we want to use as link frame. This frame should be attached to the `frameReferenceLink` link. The default value for `frameReferenceLink` is `linkName` |
| `frameReferenceLink`  | String  |  empty      | `frameReferenceLink` at which the frame is attached. If `frameReferenceLink` is empty, it will default to `linkName` |



###### Exported Frame Parameters (keys of elements of `exportedFrames`)
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `frameName`       | String |  Mandatory  | `displayName` of the frame in which the sensor measure is expressed. The selected frame should be attached to the `referenceLink` link, but `referenceLink` can be be omitted if the frameName is unique in the model.   |
| `frameReferenceLink`  | String  |  empty      | `frameReferenceLink` at which the frame is attached. If `referenceLink` is empty, it will be selected the first USERADDED frame with the specified `frameName` |
| `exportedFrameName` | String | sensorName | Name of the URDF link exported by the `exportedFrames` option |
| `additionalTransformation` | List | Empty | Additional transformation applied to the exported frame, it is expressed as [x, y, z, r, p, y] according to the semantics and units of the [SDF convention](http://sdformat.org/tutorials?tut=specify_pose&cat=specification&) for expressing poses. If the unmodified transformation of the additionalFrame is indicated as linkFrame_H_additionalFrameOld, this parameter specifies the additionalFrameOld_H_additionalFrame transform, and the final transform exported in the URDF is computed as linkFrame_H_additionalFrame = linkFrame_H_additionalFrameOld*additionalFrameOld_H_additionalFrame . If not specified it is assume to be the `[0, 0, 0, 0, 0, 0]` element and the specified frame is exported in the URDF unmodified. |

##### Mesh Parameters
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `filenameformat` |  String | %s  | Used for translating the filenames in the exported XML to the URDF filenames, using a formatting string. Example: "package://my_package//folder/%sb" - resolves to the package name and adds a "b" at the end to indicate a binary stl. |
| `forcelowercase` |  Boolean | False | Used for translating the filenames. Ff True, it forces all filenames to be lower case. |
| `scale` | String |  None | If this parameter is defined, the scale attribute of the mesh in the URDF will be set to its value. Example: "0.01 0.01 0.01" - if your meshes were saved using centimeters as units instead of meters.  |
| `stringToRemoveFromMeshFileName` | String |  None | This parameter allows to specify a string that will be removed from the mesh file names. Example: "_prt"  |
| `assignedCollisionGeometry` | Array |  None | Structure for redefining the collision geometry for a given link.  |
| `assignedColors` | Map |  {} (Empty Map) | If a link is in this map, the color found in the SimMechanics file is substituted with the one passed through this map. The color is represented by a 4 element vector of containing numbers from 0 to 1 representing the red, green, blue and alpha component.  |

###### Assigned collision geometries (keys of elements of `assignedCollisionGeometry`)
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `linkName`       | String |  Mandatory  | Name of the link for which the collision geometry is specified. |
| `geometricShape`  | Dictionary  |  Mandatory  | This dictionary contains the parameters used to define the type and the position of the geometric shape. In particular we have: <ul><li>shape: geometric shape type. Supported "box", "cylinder", "sphere". </li><li>type dependent geometric shape parameters. Refer to [SDF Geometry](http://sdformat.org/spec?elem=geometry). </li><li>origin: String defining the pose of the geometric shape with respect to the `linkFrame`. </li></ul> |

~~~
assignedCollisionGeometry:
  - linkName: r_foot
    geometricShape:
      shape: cylinder
      radius: 0.16
      length: 0.06
      origin: "0.0 0.03 0.0 1.57079632679 0.0 0.0"
  - linkName: l_foot
    geometricShape:
      shape: box
      size: 0.4 0.2 0.1
      origin: "0.0 0.0 0.0 0.0 0.0 0.0"
~~~


##### Inertia parameters
Parameters related to the inertia parameters of a link

| Attribute name | Type | Default Value | Description |
|:----------------:|:---------:|:------------:|:-------------:|
| `assignedMasses` | Map  | {} (Empty Map) | If a link is in this map, the mass found in the SimMechanics file is substituted with the one passed through this map. Furthermore, the inertia matrix present in the SimMechanics file is scaled accounting for the new mass (i.e. multiplied by new_mass/old_mass). The mass is expressed in Kg. |
| `assignedInertias`    | Array | empty | Structure for redefining the inertia tensor (at the COM) for a given link.  |

###### Assigned Inertias parameters (elements of `assignedInertias` parameters)
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:-----------:|:-------------:|
| `linkName`       | String |  Mandatory  | name of the link for which we want to set the frame |
| `xx`      | String | empty  | If defined, change the Ixx value of the inertia matrix of the link. Unit of measure: Kg*m^2 . |
| `yy`      | String | empty  | If defined, change the Iyy value of the inertia matrix of the link. Unit of measure: Kg*m^2 . |
| `zz`      | String | empty  | If defined, change the Izz value of the inertia matrix of the link. Unit of measure: Kg*m^2 . |

~~~
assignedMasses:
  link1: 1
  link2: 3

assignedInertias:
  - linkName: link1
    xx: 0.0001
  - linkName: link1
    xx: 0.0003
    yy: 0.0003
    zz: 0.0003
~~~

##### Sensors Parameters
Sensor information can be expressed using arrays of sensor options.
Note that given that the URDF still does not support an official format for expressing sensor information,
this script will output two different elements for each sensor:
* a `<gazebo>` element, necessary to simulate the sensor in Gazebo when loading the URDF, as documented in http://gazebosim.org/tutorials?tut=ros_gzplugins .
* a more URDF-like `<sensor>` element, in particular the variant supported by the iDynTree library, as documented in https://github.com/robotology/idyntree/blob/master/doc/model_loading.md .


| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `forceTorqueSensors` | Array  |  empty      | Array of option for exporting 6-Axis ForceTorque sensors |
| `sensors`            | Array  |  empty      | Array of option for exporting generic sensors (e.g. camera, depth, imu, ray..) |

###### ForceTorque Sensors Parameters (keys of elements of `forceTorqueSensors`)
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `jointName` | String  |  empty      | Name of the Joint for which this sensor measure the ForceTorque. |
| `directionChildToParent` | Bool | True | True if the sensor measures the force excerted by the child on the parent, false otherwise |
| `sensorName`      | String   | jointName | Name of the sensor, to be used in the output URDF file |
| `exportFrameInURDF` | Bool   | False        | If true, export a fake URDF link whose frame is coincident with the sensor frame (as if the sensor frame was added to the `exportedFrames` array). |
| `exportedFrameName` | String | `sensorName` if defined `jointName` otherwise | Name of the URDF link exported by the `exportFrameInURDF` option |
| `frameName`       | String |  empty  | Name of the frame in which the sensor measure is expressed. Mandatory if `exportFrameInURDF` is set to yes. |
| `linkName`         | String  |  empty      | Name of the parent link at which the sensor is rigidly attached. Mandatory if `exportFrameInURDF` is set to yes. |
| `frameReferenceLink`    | String  | `linkName`    | link at which the sensor frame is attached (to make sense, this link should be rigidly attached to the `linkName`. By default `referenceLink` is assumed to be `linkName`.
| `frame` | String | empty | The value of this element may be one of: child, parent, or sensor. It is the frame in which the forces and torques should be expressed. The values parent and child refer to the parent or child links of the joint. The value sensor means the measurement is rotated by the rotation component of the `<pose>` of this sensor. The translation component of the pose has no effect on the measurement. |
| `sensorBlobs` | String | empty | Array of strings (possibly on multiple lines) represeting complex XML blobs that will be included as child of the `<sensor>` element of type "force_torque" |

Note that for now the FT sensors sensor frame is required to be coincident with child link frame, due
to URDF limitations.

###### Generic Sensors Parameters (keys of elements of `sensors`)
| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `linkName`         | String  |  Mandatory      | Name of the Link at which the sensor is rigidly attached. |
| `frameName`        | String  |  empty      | `displayName` of the frame in which the sensor measure is expressed. The selected frame must be attached to the `referenceLink` link. If empty the frame used for the sensor is coincident with the link frame. |
| `frameReferenceLink`    | String  | linkName    | link at which the sensor frame is attached (to make sense, this link should be rigidly attached to the `linkName`. By default `referenceLink` is assumed to be `linkName`.
| `sensorName`      | String   | LinkName_FrameName | Name of the sensor, to be used in the output URDF file |
| `exportFrameInURDF` | Bool   | False        | If true, export a fake URDF link whose frame is coincident with the sensor frame (as if the sensor frame was added to the `exportedFrames` array) |
| `exportedFrameName` | String | sensorName | Name of the URDF link exported by the `exportFrameInURDF` option |
| `sensorType` | String | Mandatory | Type of sensor. Supported: "altimeter", "camera", "contact", "depth", "gps", "gpu_ray", "imu", "logical_camera", "magnetometer", "multicamera", "ray", "rfid", "rfidtag", "sonar", "wireless_receiver", "wireless_transmitter" |
| `updateRate` | String | Mandatory | Number representing the update rate of the sensor. Expressed in [Hz]. |
| `sensorBlobs` | String | empty | Array of strings (possibly on multiple lines) represeting complex XML blobs that will be included as child of the `<sensor>` element |

##### XML Blobs options
IF you use extensions of URDF, we frequently want to add non-standard tags as child of the `<robot>` root element.
Using the XMLBlobs option, you can pass an array of strings (event on multiple lines) represeting complex XML blobs that you
want to include in the converted URDF file. This will be included without modifications in the converted URDF file.
Note that every blob must have only one root element.

| Attribute name   | Type   | Default Value | Description  |
|:----------------:|:---------:|:------------:|:-------------:|
| `XMLBlobs `         | Array of String  |  [] (empty array)   | List of XML Blobs to include in the URDF file as children of `<robot>` |

#### CSV  Parameter File
Using the `--csv-joints` options it is possible to load some joint-related information from a csv
file. The rationale for using CSV over YAML for some information related to the model (for example joint limits) is to use a format that it is easier to modify  using common spreadsheet tools like Excel/LibreOffice Calc, that can be easily used also by people without a background in computer science.

##### Format
The CSV file is loaded by loaded by the python `csv` module, so every dialect supported
by the [`csv.Sniffer()`](https://docs.python.org/library/csv.html#csv.Sniffer) is automatically
supported by `simmechanics-to-urdf`.

The CSV file is formed by a header line followed by several content lines,
as in this example:
~~~
joint_name,lower_limit,upper_limit
torso_yaw,-20.0,20.0
torso_roll,-20.0,20.0
~~~

The order of the elements in header line is arbitrary, but the supported attributes
are listed in the following:

| Attribute name | Required | Unit of Measure |   Description  |
|:--------------:|:--------:|:----------------:|:---------------:|
| joint_name     |  **Yes**  |      -          | Name of the joint to which the content line is referring |
| lower_limit    |  No      | Degrees         | `lower` attribute of the `limit` child element of the URDF `joint`. **Please note that we specify this limit here in Degrees, but in the urdf it is expressed in Radians, the script will take care of  internally converting this parameter.** |
| upper_limit    |  No      | Degrees         | `upper` attribute of the `limit` child element of the URDF `joint`. **Please note that we specify this limit here in Degrees, but in the urdf it is expressed in Radians, the script will take care of  internally converting this parameter.** |
| velocity_limit | No      | Radians/second    | `velocity` attribute of the `limit` child element of the URDF `joint`. |
| effort_limit | No      |  Newton meters    | `effort` attribute of the `limit` child element of the URDF `joint`.
| damping  | No      |  Newton meter seconds / radians    | `damping` of the `dynamics` child element of the URDF `joint`. |
| friction | No      |  Newton meters    | `friction` of the `dynamics` child element of the URDF `joint`. |


### Maintainers
This repository is maintained by:

| | |
|:---:|:---:|
| [<img src="https://github.com/Nicogene.png" width="40">](https://github.com/Nicogene) | [@Nicogene](https://github.com/Nicogene) |
| [<img src="https://github.com/mfussi66.png" width="40">](https://github.com/mfussi66) | [@mfussi66](https://github.com/mfussi66) |
