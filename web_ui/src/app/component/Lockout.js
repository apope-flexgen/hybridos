import React, { useEffect, useState } from 'react';
import { useDispatch } from 'react-redux';

import {
    updatePropertyValue,
    setOrPutDataForURI,
    getDataForURI,
} from '../../AppConfig';

import { userUsername } from '../../AppAuth';

import LockIcon from '@mui/icons-material/Lock';

import LockOpenIcon from '@mui/icons-material/LockOpen';
import IconButton from '@mui/material/IconButton';

import { Tooltip } from '@mui/material';
import { setPopupAlert } from '../../actions';
/*
    props:
    baseURI
    sourceURI
    enabled (is /maint_mode)
    index
    data
*/

export function Lockout(props) {
    const dispatch = useDispatch();

    const [isLocked, setIsLocked] = useState(false);
    const [lockingUser, setLockingUser] = useState('');

    const { index, data, category, asset_id, setLockedOut } = props;

    useEffect(() => {
        // update lockout state when asset is changed or
        // when enabled changes (signaling a change in lockout state)
        getLockoutState();
    }, [index, data?.enabled]);

    /**
     * Checks if maint_mode_lockout value has changed
     */
    const getLockoutState = () => {
        const databaseUri = 'dbi/site_controller/assets';
        getDataForURI(databaseUri, false)
            .then((response) => response.json())
            .then((response) => {
                let status = null;
                let theData = response.body;
                let instances = theData.assets[category].asset_instances;
                for (let i = 0; i < instances.length; i++) {
                    const inst = instances[i];
                    if (inst.id === asset_id) {
                        status = inst.maint_mode_lockout;
                        break;
                    }
                }
                console.log(`${asset_id} lock status`, status);

                if (status && 
                    (isLocked !== status.value ||
                    lockingUser !== status.username)
                ) {
                    setIsLocked(status.value);
                    setLockingUser(status.username);
                    setLockedOut(
                        status.value && status.username !== userUsername,
                        index
                    );
                }
            });
    };

    /**
     * updates the value of maint_mode_lockout
     */
    const setLockoutState = (val, usr) => {
        const databaseUri = 'dbi/site_controller/assets';

        // send lock status to fims
        let fimsURI = `assets/${category}/${asset_id}`;
        updatePropertyValue(fimsURI, 'lock_mode', val).then((response) => {
            if (!response.ok) {
                throw new Error(
                    `${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`
                );
            }
        });

        // update lock info in DBI
        getDataForURI(databaseUri, false)
            .then((response) => response.json())
            .then((response) => {
                let body = response.body;
                let instances = body.assets[category].asset_instances;
                for (let i = 0; i < instances.length; i++) {
                    const inst = instances[i];
                    if (inst.id === asset_id) {
                        inst.maint_mode_lockout = {
                            value: val,
                            username: usr,
                        };
                        break;
                    }
                }
                setOrPutDataForURI(databaseUri, JSON.stringify(body), 'POST')
                    .then((response) => response.json())
                    .then((response) => {
                        console.log('set lock_mode dbi response', response);
                    });
            });
    };

    const confirmUnlock = () => {
        if (userUsername === lockingUser) {
            setIsLocked(false);
            setLockingUser('');
            setLockoutState(false, '');
        } else {
            dispatch(
                setPopupAlert(
                    400,
                    `Cannot change lockdown mode of asset: ${asset_id}, was locked by user: ${lockingUser}`,
                    true
                )
            );
            console.error('lock access denied', {
                userUsername: userUsername,
                'lockingUser state': lockingUser,
            });
        }
    };

    const confirmLock = () => {
        setIsLocked(true);
        setLockingUser(userUsername);
        setLockoutState(true, userUsername);
    };

    return (
        <div>
            {isLocked ? (
                <Tooltip title={`Locked by: ${lockingUser}`} placement="left">
                    <IconButton
                        data-cy={`confirmUnlock_`}
                        color="secondary"
                        onClick={confirmUnlock}
                    >
                        <LockIcon />
                    </IconButton>
                </Tooltip>
            ) : (
                <IconButton
                    data-cy={`confirmLock_`}
                    color="primary"
                    onClick={confirmLock}
                >
                    <LockOpenIcon />
                </IconButton>
            )}
        </div>
    );
}
