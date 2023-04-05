import { Injectable } from '@nestjs/common'

import { Permissions, Permission, Role } from './interfaces/permission.interface'
import * as fs from 'fs'
import * as path from 'path'

// temporarily hardcoded to get a working version, will be corrected
// when we decide how we want to read permissions
const LOCALFILEPATH = './src/permissions/permissions.json'

@Injectable()
export class PermissionsService {
    // holds the perms object in memory
    permissions: Permissions
    // flag to indicate if the permissions file has been read into memory
    initialized = false
    constructor() {
        const webServerConfigPath = process.argv[4]
        const configFilePath = fs.existsSync(webServerConfigPath)
        const filePath = configFilePath
            ? path.resolve(webServerConfigPath, 'permissions.json')
            : LOCALFILEPATH

        // read file on init
        this.readToMemory(filePath)
    }
    // public method to check if a role has access to a uri;
    // returns true if access level is greater than requested
    // permissionLevel
    ConfirmRoleAccess = (role: string, permissionLevel: string, uri: string): boolean => {
        const [accessLevel] = this.getAccessLevel(role, uri)
        const numericLevel = this.getNumericLevel(permissionLevel)
        return accessLevel >= numericLevel
    }
    // returns the numeric access level of a role for a given uri
    getAccessLevel(role: string, uri: string): number[] {
        // if the uri or role is empty, return 0, 0
        if (uri === '' || role === '') {
            return [0, 0]
        }

        // if role doesn't exist, no access, no specificity
        if (!this.permissions[role]) {
            return [0, 0]
        }

        // extract the permissions for the given role and get access
        const roleObj = this.permissions[role]
        return this.accessHelper(uri, roleObj)
    }
    // stores perms in memory, sets initialized flag to true,
    // catches errors from reading file
    readToMemory = async (filePath: string): Promise<boolean> => {
        // TODO: what do we do on error here?
        try {
            this.permissions = await this.readPermissionsFromFile(filePath)
            this.initialized = true
            return true
        } catch (e) {
            console.log(e)
            return false
        }
    }
    private accessHelper = (uri: string, roleObj: Role): number[] => {
        let curSpecificity = 0
        let curAccessLevel = 0

        // check the "/" case
        const permObj = roleObj[''] as Permission
        curAccessLevel = permObj._accessLevel

        // other cases
        const [dugAccessLevel, dugSpecificity] = this.digUri(roleObj, uri)
        if (dugSpecificity > curSpecificity) {
            curAccessLevel = dugAccessLevel
            curSpecificity = dugSpecificity
        }

        // get inherited perms next - these should only override if more specific
        const [inheritedAccessLevel, inheritedSpecificity] = this.getInheritedPerms(roleObj, uri)
        if (inheritedSpecificity > curSpecificity) {
            curAccessLevel = inheritedAccessLevel
            curSpecificity = inheritedSpecificity
        }

        return [curAccessLevel, curSpecificity]
    }
    // helper function to dig through a uri and return the access level/specificity
    private digUri = (roleObj: Role, uri: string): number[] => {
        const splitUri = uri.split('/')
        let permObj: Permission
        let curAccessLevel = 0
        let curSpecificity = 0

        if (splitUri.length > 1 && splitUri[1] != '') {
            // first split - separate because this is digging into a Role, not a Permission
            if (roleObj[splitUri[1]]) {
                permObj = roleObj[splitUri[1]] as Permission
                curAccessLevel = permObj._accessLevel
                curSpecificity = 1
            }

            // following cases - digging into a Permission
            const [dugAccessLevel, dugSpecificity] = this.digPermission(permObj, splitUri)
            if (dugSpecificity > curSpecificity) {
                curAccessLevel = dugAccessLevel
                curSpecificity = dugSpecificity
            }
        }

        return [curAccessLevel, curSpecificity]
    }
    // helper function to dig through a permission and return the access level/specificity
    private digPermission = (permObj: Permission, splitUri: string[]): number[] => {
        let curAccessLevel = 0
        let curSpecificity = 0

        // permObj will only exist if the first split was present,
        // so we can use it to check if we should continue to second split
        if (permObj) {
            for (let i = 2; i < splitUri.length; i++) {
                if (splitUri[i] != '' && permObj[splitUri[i]]) {
                    permObj = permObj[splitUri[i]] as Permission
                    curAccessLevel = permObj._accessLevel
                    curSpecificity = i
                } else {
                    break
                }
            }
        }
        return [curAccessLevel, curSpecificity]
    }
    // helper function to get inherited permissions
    private getInheritedPerms = (roleObj: Role, uri: string): number[] => {
        let curAccessLevel = 0
        let curSpecificity = 0

        if (roleObj.included != '') {
            const [inheritedAccessLevel, inheritedSpecificity] = this.getAccessLevel(
                roleObj.included,
                uri
            )
            if (inheritedSpecificity > curSpecificity) {
                curAccessLevel = inheritedAccessLevel
                curSpecificity = inheritedSpecificity
            }
        }

        return [curAccessLevel, curSpecificity]
    }
    // helper function to get the numeric level of a permission
    private getNumericLevel = (level: string): number => {
        switch (level) {
            case 'read':
                return 1
            case 'readWrite':
                return 2
            default:
                return 0
        }
    }
    // FIXME: how are we actually reading permissions?
    // helper function, reads the permissions file
    private readPermissionsFromFile = async (filePath: string): Promise<Permissions> => {
        const file = fs.readFileSync(filePath, 'utf8')
        const permissions = JSON.parse(file)
        return permissions
    }
}
