import { formatDataFunc } from '../../global'

export interface UseGetComponentDataProps {
    useGetCustomQuery?: () => any
    // eslint-disable-next-line no-unused-vars
    useGetCustomQueryWithParams?: (arg: any) => any
    queryParam?: string | object
    initialData: object | any
    formatData: formatDataFunc
}
