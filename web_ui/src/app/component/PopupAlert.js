import React, { useState, useEffect } from 'react';

import {
    Snackbar,
    Typography
} from '@mui/material';

import AlertTitle from '@mui/material/AlertTitle';
import Alert from '@mui/material/Alert';

import { useSelector } from "react-redux";
import { setPopupAlert } from '../../actions';
import { useDispatch } from "react-redux";

function PopupAlert(props) {
    const popupTitle = useSelector((state) => state.popupAlert.popupTitle);
    const popupSeverity = useSelector((state) => state.popupAlert.popupSeverity);
    const popupContent = useSelector((state) => state.popupAlert.popupContent)
    const dispatch = useDispatch();

    const handleClose = () => {
        dispatch(setPopupAlert(popupSeverity=== 'error' ? 400 : 200, popupContent, false))
    }; 

    return (
        <Snackbar
            open={useSelector((state) => state.popupAlert.isVisible)}
            onClose={handleClose}
        >
            <Alert onClose={handleClose} severity={popupSeverity} variant='filled'>
                <AlertTitle style={{ fontWeight: 'bold' }}>{popupTitle}</AlertTitle>
                <Typography>{popupContent}</Typography>
            </Alert>
        </Snackbar>
    )
    
}

export default PopupAlert;

