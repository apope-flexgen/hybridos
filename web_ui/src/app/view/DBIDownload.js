/* eslint-disable class-methods-use-this */
/* eslint-disable no-alert */
import React from 'react';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import CardHeader from '@mui/material/CardHeader';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import DropDownMenu from '../component/DropDownMenu';
import { STYLES_FEATURES } from '../styles';
import {
    getDataForURI,
    setOrPutDataForURI,
} from '../../AppConfig';

const uploadToNewDocumentString = '** upload to NEW DOCUMENT named below **';
let dataToUpload;

/**
 * Component to download from DBI
 */
class DBIDownload extends React.PureComponent {
    /**
     * Initialize data objects
     * @param {object} props 
     */
    constructor(props) {
        super(props);
        this.state = {
            collections: [],
            documentsInDownloadSelectedCollection: [],
            downloadSelectedCollection: '',
            downloadSelectedDocument: '',
            documentsInUploadSelectedCollection: [],
            uploadSelectedCollection: '',
            uploadSelectedDocument: '',
            uploadFileName: '',
        };
        this.downloadJSONFromDBI = this.downloadJSONFromDBI.bind(this);
        this.downloadJSONFromDBIToTextarea = this.downloadJSONFromDBIToTextarea.bind(this);
        this.uploadJSONToDBI = this.uploadJSONToDBI.bind(this);
        this.uploadJSONToDBIFromTextarea = this.uploadJSONToDBIFromTextarea.bind(this);
        this.selectJSONFile = this.selectJSONFile.bind(this);
    }

    /**
     * Set state based off key and value
     * @param {string} key key
     * @param {*} value value
     */
    doSetState(key, value) {
        this.setState({
            [key]: value,
        });
    }

    /**
     * Get collections from DBI
     */
    getCollectionsFromDBI() {
        getDataForURI('dbi/show_collections')
            .then((response) => response.json())
            .then((response) => {
                this.doSetState('collections', response.body);
            })
            .catch((err) => {
                throw new Error(`ERROR in DBIDownload/getCollectionsFromDBI: ${err}`);
            });
    }

    // eslint-disable-next-line camelcase
    /**
     * Mount component
     */
    UNSAFE_componentWillMount() {
        this.getCollectionsFromDBI();
        dataToUpload = '';
        // this resets dataToUpload when someone clicks away from this page and then returns
    }

    /**
     * Handle event?
     * @param {*} event
     */
    handleChange = (event) => {
        this.setState({
            [event.target.id]: event.target.value,
        });
    };

    /**
     * Get documents from DBI
     * @param {*} collection collection to pull from
     * @param {*} uploadMenu
     * @param {*} callback
     */
    getDocumentsFromDBI(collection, uploadMenu=false, callback) {
        let documentsInDownloadSelectedCollection;
        let documentsInUploadSelectedCollection;
        getDataForURI(`dbi/${collection}/show_documents`)
            .then((response) => response.json())
            .then((response) => {
                if (uploadMenu) {
                    documentsInUploadSelectedCollection = response.body;
                    documentsInUploadSelectedCollection.push(uploadToNewDocumentString);
                    this.doSetState('documentsInUploadSelectedCollection', documentsInUploadSelectedCollection);
                } else {
                    documentsInDownloadSelectedCollection = response.body;
                    this.doSetState('documentsInDownloadSelectedCollection', documentsInDownloadSelectedCollection);
                }
                if (callback) callback();
            })
            .catch((err) => {
                if (callback) callback();
                throw new Error(`ERROR in DBIDownload/getDocumentsFromDBI: ${err}`);
            });
    }

    /**
     * Get JSON from DBI
     * @param {boolean} toTextarea if true, send to text area
     */
    downloadJSONFromDBI(toTextarea) {
        document.getElementById('JSONDisplayAndEdit').value = '';
        if (this.state.downloadSelectedCollection && this.state.downloadSelectedDocument) {
            let databaseUri;
            let downloadName;
            databaseUri = `dbi/${this.state.downloadSelectedCollection}/${this.state.downloadSelectedDocument}`;
            downloadName = `${this.state.downloadSelectedDocument}.json`;
            const downloadNode = document.createElement('a');
            getDataForURI(databaseUri)
                .then((response) => response.json())
                .then((response) => {
                    const theData = response.body;
                    if (theData.error) {
                        alert(`An error occurred when trying to download:\n\n${theData.error.message}`);
                    } else if (toTextarea === true) {
                        document.getElementById('JSONDisplayAndEdit').value = JSON.stringify(theData, null, 4);
                    } else {
                        const jsonObject = `data:text/json;charset=utf-8,${encodeURIComponent(JSON.stringify(theData))}`;
                        // const downloadNode = document.createElement('a'); // for FireFox?
                        downloadNode.setAttribute('href', jsonObject);
                        downloadNode.setAttribute('download', downloadName);
                        downloadNode.click();
                    }
                    downloadNode.remove();
                });
        } else {
            alert('Please select both a Collection and a Document before clicking Download.');
        }
    }

    /**
     * Not sure why this is here, but downloads to text area
     */
    downloadJSONFromDBIToTextarea() {
        this.downloadJSONFromDBI(true);
    }

    /**
     * Drop down select?
     * @param {*} event
     */
    selectJSONFile(event) {
        const uploadFileName = event.target.files[0].name;
        this.doSetState('uploadFileName', uploadFileName);
        const fileReader = new FileReader();
        fileReader.readAsText(event.target.files[0]);
        fileReader.onload = (event2) => {
            dataToUpload = JSON.stringify(JSON.parse(event2.target.result));
            document.getElementById('dataToUpload').value = dataToUpload;
            document.getElementById('JSONDisplayAndEdit').value = JSON.stringify(JSON.parse(event2.target.result), null, 4);
        };
        this.insertUploadFileName(null, uploadFileName);
    }

    /**
     * Sets document name to upload
     * @param {*} uploadSelectedDocument
     * @param {*} uploadFileName name of document
     */
    insertUploadFileName(uploadSelectedDocument, uploadFileName) {
        const calculatedUploadSelectedDocument = uploadSelectedDocument
            || this.state.uploadSelectedDocument;
        const uploadFileNameShort = (uploadFileName || this.state.uploadFileName).replace('.json', '');
        if (calculatedUploadSelectedDocument === uploadToNewDocumentString
            && !this.state.uploadToNewDocumentName && uploadFileNameShort
            && (!this.state.documentsInUploadSelectedCollection.includes(uploadFileNameShort))) {
            this.doSetState('uploadToNewDocumentName', uploadFileNameShort);
            document.getElementById('uploadToNewDocumentName').value = uploadFileNameShort;
        } else if ((calculatedUploadSelectedDocument !== uploadToNewDocumentString)
            && this.state.uploadToNewDocumentName) {
            this.doSetState('uploadToNewDocumentName', '');
            document.getElementById('uploadToNewDocumentName').value = '';
        }
    }

    /**
     * Regex check of URI
     * @param {string} uri URI to check
     */
    checkThatURIIsClean(uri) {
        // eslint-disable-next-line no-useless-escape
        if (uri.toLowerCase().match(/^[a-z0-9_.]+$/) === null) {
            // if null, there are bad characters. if the request has any characters other than
            // alphanumeric, underscore, or period, then we will reject it
            return false;
        }
        return true;
    }

    /**
     * Upload JSON to DBI
     * @param {boolean} fromTextarea true if JSON supplied from text area
     */
    uploadJSONToDBI(fromTextarea) {
        if (this.state.documentsInUploadSelectedCollection
            .includes(this.state.uploadToNewDocumentName)
            && (this.state.uploadSelectedDocument === uploadToNewDocumentString)) {
            alert('ERROR: the New Document Name you have entered is the same as an existing document. If you want to override the existing document, please select it from the list above.');
        } else if (this.state.uploadSelectedCollection && this.state.uploadSelectedDocument
            && (this.state.uploadSelectedDocument !== uploadToNewDocumentString
                || ((this.state.uploadSelectedDocument === uploadToNewDocumentString)
                    && this.state.uploadToNewDocumentName))) {
            let databaseUri;
            if ((this.state.uploadSelectedDocument === uploadToNewDocumentString)
                && this.state.uploadToNewDocumentName) {
                databaseUri = `dbi/${this.state.uploadSelectedCollection}/${this.state.uploadToNewDocumentName}`;
            } else {
                databaseUri = `dbi/${this.state.uploadSelectedCollection}/${this.state.uploadSelectedDocument}`;
            }
            if ((this.state.uploadSelectedDocument === uploadToNewDocumentString)
                && !this.checkThatURIIsClean(this.state.uploadToNewDocumentName)) {
                alert('ERROR: the New Document Name contains invalid characters. The only valid characters are alphanumeric, underscores, and periods.');
            } else {
                let body;
                if (fromTextarea === true) {
                    body = document.getElementById('JSONDisplayAndEdit').value;
                } else {
                    body = dataToUpload;
                }
                if (typeof (JSON.parse(body)) !== 'object') {
                    alert('ERROR: the data to upload must be a valid object.');
                } else {
                    // express in web_server doesn't handle 'set' the same as dbi does. We have to
                    // use the 'post' method to make a 'set' happen in dbi.
                    const setOrPut = this.state.uploadSelectedDocument === uploadToNewDocumentString ? 'post' : 'put';
                    setOrPutDataForURI(databaseUri, body, 'post')
                        .then((response) => response.json())
                        .then((response) => {
                            let responseMatch;
                            if (fromTextarea === true) {
                                responseMatch = JSON.stringify(response)
                                    === JSON.stringify(JSON.parse(body));
                            } else {
                                responseMatch = JSON.stringify(response) === dataToUpload;
                            }
                            alert(`${responseMatch ? 'SUCCESS: The data sent matches the response from dbi.'
                                : 'ERROR: The data sent DOES NOT match the response from dbi.\nThe response from dbi can be viewed in the browser console.'}`);
                            if (!responseMatch) console.log(JSON.stringify(response));
                            // then reset the documents menus:
                            this.getDocumentsFromDBI(this.state.uploadSelectedCollection,
                                true, () => {
                                    this.doSetState('uploadSelectedDocument', uploadToNewDocumentString);
                                    this.doSetState('uploadToNewDocumentName', '');
                                });
                            if (this.state.downloadSelectedCollection) {
                                this.getDocumentsFromDBI(this.state.downloadSelectedCollection);
                            }
                        });
                }
            }
        } else {
            alert('Please select both a Collection and a Document before clicking Upload.\n\nIf you want to upload to a new document, you must also enter the new document name in the field below the Overwrite Document selection.');
        }
    }

    /**
     * Not sure why this is needed, uploads JSON to DBI as true
     */
    uploadJSONToDBIFromTextarea() {
        this.uploadJSONToDBI(true);
    }

    render() {
        // these constants control the display of the New Document Name field, enabling or
        // disabling it, and making the background red if the text in the field is the same
        // as an existing document name.
        const newDocumentNameAlreadyExists = this.state.documentsInUploadSelectedCollection
            .includes(this.state.uploadToNewDocumentName);
        const disabledOrNot = this.state.uploadSelectedDocument === uploadToNewDocumentString
            ? '#ffffff' : '#eeeeee';
        const backgroundColor = newDocumentNameAlreadyExists ? '#ff9999' : disabledOrNot;
        const newDocumentNameFieldDisabled = this.state.uploadSelectedDocument
            !== uploadToNewDocumentString;

        const downloadDisabling = this.state.downloadSelectedDocument === '' || this.state.downloadSelectedCollection === '';
        const uploadDisabling = this.state.uploadSelectedCollection === '' || (this.state.uploadSelectedDocument === '' || (this.state.uploadSelectedDocument === uploadToNewDocumentString && (this.state.uploadToNewDocumentName === '' || !this.state.uploadToNewDocumentName)));

        return <>
            <Grid item md={12}>
                <Card>
                    <CardHeader
                        title='DBI Download'
                        subheader='Download data to a file or to the editable textarea below'
                    />
                    <CardContent>
                        <Grid container item md={12}>
                            <Grid item md={3} lg={3}>
                                <DropDownMenu
                                    value={this.state.downloadSelectedCollection}
                                    menuItems={this.state.collections}
                                    getStateFromChild={(selected) => {
                                        this.doSetState('downloadSelectedCollection', selected.target.value);
                                        this.getDocumentsFromDBI(selected.target.value);
                                    }}
                                    label='Download from Collection'
                                    helperText='select one'
                                />
                            </Grid>
                            <Grid item md={2} lg={2}></Grid>
                            <Grid item md={3} lg={3}>
                                <DropDownMenu
                                    value={this.state.downloadSelectedDocument}
                                    menuItems={this.state.documentsInDownloadSelectedCollection}
                                    getStateFromChild={(selected) => {
                                        this.doSetState('downloadSelectedDocument', selected.target.value);
                                    }}
                                    label='Download Document'
                                    helperText='select one'
                                />
                            </Grid>
                            <Grid item md={2} lg={2}></Grid>
                            <Grid item md={2} lg={2}>
                                <Button
                                    id='downloadJSONFromDBI'
                                    onClick={this.downloadJSONFromDBI}
                                    fullWidth={true}
                                    variant='contained'
                                    color='primary' // this is the bg color of the button
                                    disabled={downloadDisabling}
                                >
                                    Download to File
                                </Button>
                                <br />
                                <br />
                                <Button
                                    id='downloadJSONFromDBIToTextarea'
                                    onClick={this.downloadJSONFromDBIToTextarea}
                                    variant='contained'
                                    color='primary' // this is the bg color of the button
                                    disabled={downloadDisabling}
                                >
                                    Download to Textarea
                                </Button>
                            </Grid>
                        </Grid>
                    </CardContent>
                </Card>
            </Grid>
            <br />
            <Grid item md={12}>
                <Card>
                    <CardHeader
                        title='DBI Upload'
                        subheader='Upload data from a selected file or from the editable textarea below'
                    />
                    <CardContent>
                        <Grid container md={12}>
                            <Grid item md={1} lg={1}>
                                <Button
                                    id='fileSelector'
                                    onChange={this.selectJSONFile}
                                    component='label'
                                    variant='contained'
                                >
                                    <input
                                        type='file'
                                    />
                                </Button>
                            </Grid>
                        </Grid>
                        <br />
                        <br />
                        <Grid container md={12}>
                            <Grid item md={3} lg={3}>
                                <DropDownMenu
                                    value={this.state.uploadSelectedCollection}
                                    menuItems={this.state.collections}
                                    getStateFromChild={(selected) => {
                                        this.doSetState('uploadSelectedCollection', selected.target.value);
                                        this.getDocumentsFromDBI(selected.target.value, true);
                                    }}
                                    label='Upload to Collection'
                                    helperText='select one'
                                />
                            </Grid>
                            <Grid item md={2} lg={2}></Grid>
                            <Grid item md={3} lg={3}>
                                <DropDownMenu
                                    value={this.state.uploadSelectedDocument}
                                    menuItems={this.state.documentsInUploadSelectedCollection}
                                    getStateFromChild={(selected) => {
                                        this.insertUploadFileName(selected.target.value);
                                        this.doSetState('uploadSelectedDocument', selected.target.value);
                                    }}
                                    label='Overwrite Document'
                                    helperText='select one above'
                                />
                                <br />
                                <br />
                                <TextField
                                    id='uploadToNewDocumentName'
                                    onChange={this.handleChange}
                                    value={this.state.uploadToNewDocumentName}
                                    variant='standard'
                                    style={{ backgroundColor }}
                                    disabled={newDocumentNameFieldDisabled}
                                    label={this.state.uploadToNewDocumentName ? '' : 'New Document Name'}
                                    helperText='If uploading to a new document, this is the name that will be used for the new document. You can edit this as necessary. If this field is red, the name you have entered is the name of an already-existing document and cannot be used.'
                                />
                            </Grid>
                            <Grid item md={2} lg={2}></Grid>
                            <Grid item md={2} lg={2}>
                                <Button
                                    id='uploadJSONToDBI'
                                    onClick={this.uploadJSONToDBI}
                                    variant='contained'
                                    color='primary' // this is the bg color of the button
                                    disabled={this.state.uploadFileName === '' || uploadDisabling}
                                >
                                    Upload from Selected File
                                </Button>
                                <br />
                                <br />
                                <Button
                                    id='uploadJSONToDBIFromTextarea'
                                    onClick={this.uploadJSONToDBIFromTextarea}
                                    variant='contained'
                                    color='primary' // this is the bg color of the button
                                    // disabled={this.state.uploadSelectedCollection === '' || this.state.uploadSelectedDocument === ''}
                                    disabled={uploadDisabling}
                                >
                                    Upload from Textarea
                                </Button>

                            </Grid>
                        </Grid>
                        <br />
                        <br />
                        <hr></hr>
                        <br />
                        <Grid container md={12}>
                            <Grid item md={12} lg={12}>
                                <TextField
                                    id='dataToUpload'
                                    onChange={this.handleChange}
                                    value={dataToUpload}
                                    InputProps={{
                                        style: {
                                            fontFamily: '"Courier New", Courier, monospace', fontSize: '1em',
                                        },
                                    }}
                                    disabled={true}
                                    fullWidth={true}
                                    variant='outlined'
                                    helperText='This is the data from the selected file (not editable here)'
                                />
                            </Grid>
                        </Grid>
                        <br />
                        <hr></hr>
                        <br />
                        <Grid container md={12}>
                            <Grid item md={12} lg={12}>
                                <TextField
                                    id='JSONDisplayAndEdit'
                                    multiline
                                    InputProps={{
                                        style: {
                                            fontFamily: '"Courier New", Courier, monospace', fontSize: '1em',
                                        },
                                    }}
                                    rows={5}
                                    maxRows={Infinity}
                                    fullWidth={true}
                                    variant='outlined'
                                    helperText='This is the textarea for displaying and editing JSON data'
                                />
                            </Grid>
                        </Grid>
                    </CardContent>
                </Card>
            </Grid>
        </>;
    }
}
export default withStyles(DBIDownload, STYLES_FEATURES);
