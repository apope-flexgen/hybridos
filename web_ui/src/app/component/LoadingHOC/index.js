import React, { useState } from 'react';
import ActivityIndicator from '../ActivityIndicator';

export const LoadingHOC = (WrappedComponent) => {
    function HOC(props) {
        const isLoginPage = WrappedComponent.toString().includes('LoginPage')
        const[isLoading, setLoading] = useState(true);

        const setLoadingState = isComponentLoading => {
            setLoading(isComponentLoading)
        }
        return(
            <>
            {isLoading && <div style={{display: 'flex', justifyContent:'center', alignItems: 'center'}}><ActivityIndicator  leftMargin={isLoginPage ? '47%' : '55%'} topMargin={'25%'} color='#000000'/></div>}
                <WrappedComponent {...props} setLoading={setLoadingState} />
            </>
        )
    }
    return HOC;
};

export default LoadingHOC;