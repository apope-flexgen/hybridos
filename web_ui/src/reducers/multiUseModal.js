const multiUseModal = (
    state={
            isVisible: false,
            modalType: '',
            submit: 'none'
        }, 
        action
    )=>{
    switch(action.type){
        case "SHOW_MULTI_USE_MODAL":
            return {...state, isVisible: action.isVisible, modalType: action.modalType};
        case "SUBMIT_MULTI_USE_MODAL":
            return {...state, submit: action.submit};
        default:
            return state;
    }
}

export default multiUseModal;