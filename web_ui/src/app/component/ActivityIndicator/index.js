/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React, { Fragment } from 'react';
import CircularProgress from '@mui/material/CircularProgress';
import Popper from '@mui/material/Popper';

class ActivityIndicator extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            
        };
    }

    render() {
        return (
            <Fragment>
                <Popper open={true} style={{marginTop: this.props.topMargin, marginLeft: this.props.leftMargin, color: this.props.color}}>
                    <CircularProgress  size={100}/>
                </Popper>
            </Fragment>
        );
    }
}

export default ActivityIndicator;
