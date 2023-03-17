import React from 'react';
import { Modal, Paper } from "@mui/material";
import MaintenanceModal from '../ModalPresets/MaintenanceModal'
import { useSelector } from "react-redux";

function MultiUseModal() {
    const type = useSelector((state) => state.multiUseModal.modalType);
    let modal;
    switch (type) {
        case 'maint_modal':
            modal = <MaintenanceModal />;
            break;
        default:
    }
    return (
    <Modal open={useSelector((state) => state.multiUseModal.isVisible)}
        style={{display:"flex", alignItems: "center", justifyContent: "center"}}>
        <Paper style={{minWidth: 550, minHeight: 200, display: "flex", flexDirection: "column", alignItems: "center"}}>
            {modal}
        </Paper>
    </Modal>);
}

export default MultiUseModal;
