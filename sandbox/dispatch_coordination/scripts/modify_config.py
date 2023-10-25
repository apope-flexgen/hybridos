#!/bin/python3

import os
import json


# load_file() is unused as the design of creating a temp file changes the inode and breaks the docker synching
# via volume mount, which means the same file cannot be used between two containers
# Preserving this temp file functionality if it's useful for local testing
# Backup then load file from path into memory for editing
def load_file(file_path):
    if not os.path.exists(file_path):
        return
    # Backup existing file
    backup_file_path = os.path.join(os.path.dirname(file_path) + "/temp_" + os.path.basename(file_path))
    os.rename(file_path, backup_file_path)
    # Open new file
    f = open(backup_file_path, "r")
    file_contents = json.load(f)
    f.close()
    return file_contents


# Recursively parse json until the key is found then replace the value if indicated by the replace_value flag
def recursive_replace(obj, key, value, associated_key, associated_value, replace_value):
    # Searching through single object
    if isinstance(obj, dict):
        for current_key, current_value in obj.items():
            # Match if either the key is found and there is no associated key/value pair, or both the key and its associated key/value pair are found
            # See update_variable function comment for an example of the associated key/value pair
            if current_key == key and (associated_key == None or (associated_key in obj.keys() and associated_value == obj[associated_key])):
                if replace_value:
                    # Remove the entire json subobject
                    if value == "delete":
                        del obj[current_key]
                    # Replace the value
                    else:
                        obj[current_key] = value
                return current_value
    # Searching through array of objects
    elif isinstance(obj, list):
        for current_item in obj:
            current_value = recursive_replace(current_item, key, value, associated_key, associated_value, replace_value)
            if current_value != None:
                if replace_value and value == "delete":
                    obj.remove(current_item)
                return current_value


# Update/delete each of the given variables
# If there are multiple possible paths an associated key/value pair can be provided
# to indicate which path to take at the ambiguous json level e.g.
#   {
#       "component_id": "ess_real_hs",
#       "variables": {},
#   },
#   {
#       "component_id": "ess_real_ls",
#       "variables": {},
#   }
# could specify the second variables array by indicating component_id "ess_real_ls
# In practice, this is done with a comma after the key, followed by a key=value e.g.
#   .../variables,component_id=ess_real_hs
# To delete a variable, simply supply the value "delete" as the replacement value
def update_variables(file_path, variables):
    # file_contents = load_file(file_path)
    f = open(file_path, "r")
    file_contents = json.load(f)

    # Update each variable
    for uri, value in variables.items():
        # Search for each URI fragment in the config, one at a time
        frags = uri.split("/")
        ptr = file_contents
        for frag in frags[1:]:
            # Any associated parsing for ambiguous path
            # Variable fragment
            associated_frags = frag.split(",")
            if len(associated_frags) == 2:
                # Associated key value pair for the variable
                associated_key_value = associated_frags[1].split("=")
            else:
                associated_key_value = [None, None]

            # Normal parse
            if frag != frags[len(frags) - 1]:
                ptr = recursive_replace(ptr, associated_frags[0], value, associated_key_value[0], associated_key_value[1], False)
            # Last frag replace
            else:
                replaced_value = recursive_replace(ptr, associated_frags[0], value, associated_key_value[0], associated_key_value[1], True)
                if replaced_value == None:
                    print("Failed to find and replace ", uri)
                    # return revert_file(file_path)
                    return

    return save_file(file_path, file_contents)


# Save file contents with all modifications
def save_file(file_path, file_contents):
    f = open(file_path, "w+")
    f.write(json.dumps(file_contents))
    f.close()


# revert_file() is unused as the design of creating a temp file changes the inode and breaks the docker synching
# via volume mount, which means the same file cannot be used between two containers
# Preserving this temp file functionality if it's useful for local testing
# Revert file to its unmodified state
def revert_file(file_path):
    backup_file_path = os.path.join(file_path, os.path.dirname(file_path) + "/temp_" + os.path.basename(file_path))
    if not os.path.exists(backup_file_path):
        return

    os.remove(file_path)
    os.rename(backup_file_path, file_path)


# Example usage
# update_variables("/usr/local/etc/config/site_controller/assets.json",
#                  {"/assets/feeders/asset_instances/rated_active_power_kw,id=feed_3": 1,
#                   "/assets/ess/asset_instances/status_type,id=ess_1": "Bit_Field",
#                   "/assets/ess/asset_instances/components,id=ess_2/variables,component_id=ess_real_ls/system_chargeable_energy/scaler": 10})
# update_variables("/usr/local/etc/config/site_controller/sequences.json",
#                  {"/sequences/Startup/paths/steps,return_id=RunMode1/entry_actions,step_name=Start Solar": "delete"})
# # Run tests
# revert_file("/usr/local/etc/config/site_controller/assets.json")
# revert_file("/usr/local/etc/config/site_controller/sequences.json")
