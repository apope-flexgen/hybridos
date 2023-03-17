import React, { Component } from 'react';
import { TextField, Select, Button, MenuItem, Typography, FormControl, InputLabel } from "@mui/material";
import { setPopupAlert } from '../../../actions/index';
import { useDispatch } from "react-redux";
import { showMultiUseModal, submitMultiUseModal } from '../../../actions/index';

import {
    userRole, userUsername,
} from '../../../AppAuth';

import { setOrPutDataForURI } from '../../../AppConfig';


import { useSelector } from "react-redux";

function sendAuditLog(reason, comments){
    let body = {};
    body.reason = reason;
    body.comments = comments;
    body.created = Date.now();
    body.username = userUsername;
    body.userrole = userRole;
    body.modified_field = "maintenance_mode";
    body.modified_value = true;
    let bodyString = JSON.stringify(body);
    console.log(bodyString)
    const uri = `dbi/audit/audit_log_${Date.now()}`
    setOrPutDataForURI(uri, bodyString, 'POST')
        .catch(e=>{
            console.log(e);
        })
}

function MaintenanceModal() {
    const [reason, setReason] = React.useState('Test power commands');
    const [text, setText] = React.useState('');
    const dispatch = useDispatch();

    const handleChange = (event) => {
        setReason(event.target.value);
    };
    const handleText = (event) => {
        setText(event.target.value);
    };
    const handleCancel = () => {
        dispatch(submitMultiUseModal('cancel'));
        clearData();
    };

    const handleSubmit = () => {
        if(!text.length > 0){ 
            dispatch(setPopupAlert(400, 'Invalid entry: Comment must not be empty', true))
            return;
        } 
        dispatch(submitMultiUseModal('success'))
        sendAuditLog(reason, text)
        clearData();
    }  

    const clearData = () => {
        dispatch(showMultiUseModal(false, 'maint_modal'))
        setReason('Test power commands');
        setText('');
    }

    return (<>
                <Typography style={{marginTop: 20, width: "100%", textAlign: "center", fontSize: '24px', fontWeight: 'bold', padding: 10}}>
                    Enable Maintenance Mode
                </Typography>
                <div style={{display:"flex", alignItems: "center", justifyContent: "center", flexDirection: "column", margin: 20}}>
                <FormControl style={{margin: 20}} variant="standard">
                    <InputLabel>Reason</InputLabel>
                        <Select
                            value={reason}
                            label="Reason"
                            onChange={handleChange}
                            style={{minWidth: 550}}
                        >
                            <MenuItem value={'Test power commands'}>Test power commands</MenuItem>
                            <MenuItem value={'Open / Close contactors'}>Open / Close contactors</MenuItem>
                            <MenuItem value={'Isolate unit from system'}>Isolate unit from system</MenuItem>
                        </Select>
                </FormControl>
                <FormControl style={{margin: 20}}>
                    <TextField style={{minWidth: 550}} placeholder="Comment" value={text} onChange={handleText} variant="standard" />
                </FormControl>
                <div style={{margin: 5, alignItems: 'right'}}>
                    <Button style={{margin: 10, backgroundColor: 'rgba(0, 0, 0, 0.31)',}} variant="contained" onClick={handleCancel}>Cancel</Button>
                    <Button style={{margin: 10}} variant="contained" onClick={handleSubmit}>Submit</Button>
                </div>
                </div>
    </>)
    
}

export default MaintenanceModal;
