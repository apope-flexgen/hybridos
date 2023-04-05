import { formatDataFunc } from '../../../global'

/* This is the function that we want the useGetComponentData to use to format our data, from a template (initalData) */
const formatData: formatDataFunc = (initialData, data) => {
    const newData = initialData
    newData.configData = data
    return newData
}

export default formatData
