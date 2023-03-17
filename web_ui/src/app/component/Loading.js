import React from 'react';

import CircularProgress from '@mui/material/CircularProgress';

function Loading(props) {
    return (
        <div style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100%', width: '100%', marginBottom: '10px', ...props.styling }}>
            <CircularProgress size={props.size} />
        </div>
    );
}

export default Loading;
