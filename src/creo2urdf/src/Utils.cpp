/*
 * Copyright (C) 2006-2023 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <creo2urdf/Utils.h>

/**
 * @brief Computes the unit vector of a Creo Axis. 
 * The axis is defined by start and end point, and the magnitude is normalized. 
 * 
 * @param axis_data The axis datum as a CurveDescriptor
 * @return std::array<double, 3> The resulting normalized unit vector
 */
std::array<double, 3> computeUnitVectorFromAxis(pfcCurveDescriptor_ptr axis_data)
{
    auto axis_line = pfcLineDescriptor::cast(axis_data); // cursed cast from hell

    // There are just two points in the array
    pfcPoint3D_ptr pstart = axis_line->GetEnd1();
    pfcPoint3D_ptr pend = axis_line->GetEnd2();

    std::array<double, 3> unit_vector = { 0, 0, 0 };

    double module = sqrt(pow(pend->get(0) - pstart->get(0), 2) +
        pow(pend->get(1) - pstart->get(1), 2) +
        pow(pend->get(2) - pstart->get(2), 2));

    if (module < epsilon)
    {
        return unit_vector;
    }

    unit_vector[0] = (pend->get(0) - pstart->get(0)) / module;
    unit_vector[1] = (pend->get(1) - pstart->get(1)) / module;
    unit_vector[2] = (pend->get(2) - pstart->get(2)) / module;

    return unit_vector;
}

/**
 * @brief Converts a 3D Creo transform into iDynTree transform. The position is defined as a 3-elements vector in m, 
 * and rotation is represented as a SO(3) matrix for both representations.
 * 
 * @param creo_trf The 3D transform to convert
 * @param scale The factor used to scale the position vector (e.g. from m to mm)
 * @return iDynTree::Transform 
 */
iDynTree::Transform fromCreo(pfcTransform3D_ptr creo_trf, const array<double, 3>& scale)
{
    iDynTree::Transform idyn_trf;
    auto o = creo_trf->GetOrigin();
    auto m = creo_trf->GetMatrix();
    idyn_trf.setPosition({ o->get(0) * scale[0], o->get(1) * scale[1], o->get(2) * scale[2]});
    idyn_trf.setRotation({ m->get(0,0), m->get(1,0), m->get(2,0),
                           m->get(0,1), m->get(1,1), m->get(2,1),
                           m->get(0,2), m->get(1,2), m->get(2,2) });

    return idyn_trf;
}

/**
 * @brief Get the datums defined in a Solid depending on the selected type
 * 
 * @param solid The solid in which to query the datums
 * @param type The type of datums to extract as model items
 * @return std::vector<string> A vector of datum names
 */
std::vector<string> getSolidDatumNames(pfcSolid_ptr solid, pfcModelItemType type)
{
    std::vector<string> result;
    auto items = solid->ListItems(type);
    if (items->getarraysize() == 0) {
        printToMessageWindow("There is no axis in " + string(solid->GetFullName()), c2uLogLevel::WARN);
        return result;
    }

    for (int i = 0; i < items->getarraysize(); i++)
    {
        result.push_back(std::string(items->get(i)->GetName()));
    }

    return result;
}

/**
 * @brief Prints a string to the message window on the bottom part of the Creo Parametric UI.
 * The message can have different log levels, represented by an icon on its left side.
 * The available log levels are defined in text/usascii/creo2urdf.txt. 
 * 
 * @param message The desired message to be printed
 * @param log_level The desired log level. Can be NONE, INFO, WARN, PROMPT. 
 * The PROMPT enum requires user input to proceed. The user input is not processed yet. 
 */
void printToMessageWindow(std::string message, c2uLogLevel log_level)
{
    pfcSession_ptr session_ptr = pfcGetProESession();
    xstringsequence_ptr msg_sequence = xstringsequence::create();
    msg_sequence->append(xstring(message.c_str()));
    session_ptr->UIClearMessage();
    session_ptr->UIDisplayMessage("creo2urdf.txt", log_level_key.at(log_level).c_str(), msg_sequence);
}

/**
 * @brief Prints to the message window a Creo 3D transform in both origin and orientation 
 * using the iDynTree representation
 * 
 * @param m The Creo 3D transform to print
 */
void printTransformMatrix(pfcTransform3D_ptr m)
{
    printToMessageWindow(fromCreo(m).toString());
}

/**
 * @brief Prints to the message window a Creo 3D rotation matrix in SO(3)
 * 
 * @param m The Creo 3D transform to print
 */
void printRotationMatrix(pfcMatrix3D_ptr m)
{
    printToMessageWindow(to_string(m->get(0, 0)) + " " + to_string(m->get(0, 1)) + " " + to_string(m->get(0, 2)));
    printToMessageWindow(to_string(m->get(1, 0)) + " " + to_string(m->get(1, 1)) + " " + to_string(m->get(1, 2)));
    printToMessageWindow(to_string(m->get(2, 0)) + " " + to_string(m->get(2, 1)) + " " + to_string(m->get(2, 2)));
}

/**
 * @brief Replaces the first 5 bytes of a binary STL file with the string "robot".
 * This is necessary to avoid accidental parsing of the file as ASCII.
 * For details, see https://github.com/icub-tech-iit/creo2urdf/issues/16
 * 
 * @param stl Path of the STL file to edit
 */
void sanitizeSTL(std::string stl)
{
    size_t n_bytes = 5;
    char placeholder[6] = "robot";
    std::ofstream output(stl, std::ios::binary | std::ios::out | std::ios::in);

    for (size_t i = 0; i < n_bytes; i++)
    {
        output.seekp(i);
        output.write(&placeholder[i], 1);
    }
    output.close();
}

std::pair<bool, iDynTree::Transform> getTransformFromRootToChild(pfcComponentPath_ptr comp_path, pfcModel_ptr modelhdl, const std::string& link_frame_name, const array<double, 3>& scale) {
    
    iDynTree::Transform H_child = iDynTree::Transform::Identity();

    auto asm_csys_H_csys = fromCreo(comp_path->GetTransform(xtrue), scale);
    iDynTree::Transform csys_H_child;

    bool ret = false;
    std::tie(ret, csys_H_child) = getTransformFromPart(modelhdl, link_frame_name, scale);

    if (!ret)
    {
        printToMessageWindow("Unable to get the transform from to the root for " + string(modelhdl->GetFullName()), c2uLogLevel::WARN);
        return make_pair(false, H_child);
    }

    H_child = asm_csys_H_csys * csys_H_child;

    return make_pair(true, H_child);

}

/**
 * @brief Get the Transform From Part object
 * 
 * @param modelhdl 
 * @param link_frame_name 
 * @param scale 
 * @return std::pair<bool, iDynTree::Transform> 
 */
std::pair<bool, iDynTree::Transform> getTransformFromPart(pfcModel_ptr modelhdl, const std::string& link_frame_name, const array<double, 3>& scale) {

    iDynTree::Transform H_child;
    auto csys_list = modelhdl->ListItems(pfcModelItemType::pfcITEM_COORD_SYS);

    auto link_child_name = string(modelhdl->GetFullName());
    
    if (csys_list->getarraysize() == 0) {
        printToMessageWindow("There are no CSYS in the part " + link_child_name, c2uLogLevel::WARN);

        H_child = iDynTree::Transform::Identity();

        return { false, H_child };
    }

    for (size_t i = 0; i < csys_list->getarraysize(); i++)
    {
        auto csys_elem = csys_list->get(xint(i));

        auto csys = pfcCoordSystem::cast(csys_elem);

        if (string(csys->GetName()) != link_frame_name)
        {
            continue;
        }

        auto trf = csys->GetCoordSys();

        auto m = trf->GetMatrix();
        auto o = trf->GetOrigin();

        H_child = fromCreo(trf, scale);

        /*
        printToMessageWindow("csys name " + string(csys->GetName()));
        printToMessageWindow("origin x: " + to_string(o->get(0)) + " y: " + to_string(o->get(1)) + " z: " + to_string(o->get(2)));
        printToMessageWindow("transform:");
        printToMessageWindow(to_string(m->get(0, 0)) + " " + to_string(m->get(0, 1)) + " " + to_string(m->get(0, 2)));
        printToMessageWindow(to_string(m->get(1, 0)) + " " + to_string(m->get(1, 1)) + " " + to_string(m->get(1, 2)));
        printToMessageWindow(to_string(m->get(2, 0)) + " " + to_string(m->get(2, 1)) + " " + to_string(m->get(2, 2)));
        */
        //printToMessageWindow(string(csys_feat->GetFeatTypeName()));

        return { true, H_child };
    }

    return { false, H_child };
}

/**
 * @brief Get the Rotation Axis From Part object
 * 
 * @param modelhdl 
 * @param axis_name 
 * @param link_frame_name 
 * @param scale 
 * @return std::pair<bool, iDynTree::Direction> 
 */
std::pair<bool, iDynTree::Direction> getAxisFromPart(pfcModel_ptr modelhdl, const std::string& axis_name, const string& link_frame_name, const array<double, 3>& scale) {

    iDynTree::Direction axis_unit_vector;
    axis_unit_vector.zero();

    auto axes_list = modelhdl->ListItems(pfcModelItemType::pfcITEM_AXIS);
    if (axes_list->getarraysize() == 0) {
        printToMessageWindow("There is no AXIS in the part " + string(modelhdl->GetFullName()), c2uLogLevel::WARN);

        return { false, axis_unit_vector };
    }

    if (axis_name.empty())
        return { false, axis_unit_vector };

    pfcAxis* axis = nullptr;

    for (size_t i = 0; i < axes_list->getarraysize(); i++)
    {
        auto axis_elem = pfcAxis::cast(axes_list->get(xint(i)));
        if (string(axis_elem->GetName()) == axis_name)
        {
            axis = axis_elem;
        }
    }

    auto axis_data = wfcWAxis::cast(axis)->GetAxisData();

    auto axis_line = pfcLineDescriptor::cast(axis_data); // cursed cast from hell

    auto unit = computeUnitVectorFromAxis(axis_line);

    axis_unit_vector.setVal(0, unit[0]);
    axis_unit_vector.setVal(1, unit[1]);
    axis_unit_vector.setVal(2, unit[2]);

    auto& csys_H_child = getTransformFromPart(modelhdl, link_frame_name, scale).second;

    axis_unit_vector = csys_H_child.inverse() * axis_unit_vector;  // We might benefit from performing this operation directly in Creo
    axis_unit_vector.Normalize();
    
    return { true, axis_unit_vector };
}
