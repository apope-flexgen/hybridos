import React, { Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import SingleFims from '../component/SingleFIMS';
import { STYLES_FEATURES } from '../styles';

/**
 * Component for rendering FIMS
 */
class FIMSPage extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {};
    }

    // eslint-disable-next-line class-methods-use-this
    render() {
        return (
            <>
                <Fragment>
                    <SingleFims />
                </Fragment>
            </>
        );
    }
}
export default withStyles(FIMSPage, STYLES_FEATURES);
