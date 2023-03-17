const popupAlert = (
    state={
            popupTitle: "", 
            popupSeverity: "",
            popupContent: "",
            isVisible: false
        }, 
        action
    )=>{
    switch(action.type){
        case "SET_POPUP_ALERT":
            return {...state, popupTitle: action.title, popupSeverity: action.severity, popupContent: action.content, isVisible: action.isVisible};
        default:
            return state;
    }
}

export default popupAlert;