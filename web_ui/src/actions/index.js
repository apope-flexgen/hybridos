export const setPopupAlert = (status, content, isVisible) => {
    let title, severity;
    if(status==200) title='Success', severity='success';
    if(status==400) title='Failure', severity='error';
    return {
        type: "SET_POPUP_ALERT",
        payload: status,
        title, severity, content, isVisible
    }
}

export const showMultiUseModal = (isVisible, modalType) => {
    return {
        type: "SHOW_MULTI_USE_MODAL",
        isVisible, modalType
    }
}

export const submitMultiUseModal = (submit) => {
    return {
        type: "SUBMIT_MULTI_USE_MODAL",
        submit
    }
}