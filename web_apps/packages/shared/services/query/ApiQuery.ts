/* eslint-disable max-lines */
import HttpService from './HttpService'

let instance: ApiQuery

export class ApiResponse {
    status: number
    ok: boolean
    body: any
    constructor({ status, ok, body }: any) {
        this.status = status
        this.ok = ok
        this.body = body
    }
}

// FIXME: we should probably have a defined structure for
// the response object, but for now we'll just use "any"
export default class ApiQuery {
    DEFAULT_HTTP_STATUS_CODE = 500
    constructor() {
        if (!instance) {
            instance = this
        }

        return instance
    }
    getItems: (url: string, callback: (data: any) => void) => void = (
        url: string,
        callback: (data: any) => void
    ) => {
        HttpService.get(url).then(
            (response: Response) => {
                response
                    .json()
                    .then((json: any) => {
                        let apiResponse = new ApiResponse({ status: response.status, body: json })
                        if (response.ok) {
                            apiResponse.ok = true
                        } else {
                            console.error(response)
                            apiResponse.ok = false
                        }
                        callback(apiResponse)
                    })
                    .catch((error) => {
                        console.error(error)
                        callback(
                            new ApiResponse({
                                status: this.DEFAULT_HTTP_STATUS_CODE,
                                ok: false,
                                body: error,
                            })
                        )
                    })
            },
            (error) => {
                console.error(error)
                callback(
                    new ApiResponse({
                        status: this.DEFAULT_HTTP_STATUS_CODE,
                        ok: false,
                        body: error,
                    })
                )
            }
        )
    }
    get: (url: string, callback: (data: any) => void) => void = (
        url: string,
        callback: (data: any) => void
    ) => {
        HttpService.get(url).then(
            (response: Response) => {
                response
                    .json()
                    .then((json: any) => {
                        let apiResponse = new ApiResponse({ status: response.status, body: json })
                        if (response.ok) {
                            apiResponse.ok = true
                        } else {
                            console.error(response)
                            apiResponse.ok = false
                        }
                        callback(apiResponse)
                    })
                    .catch((error) => {
                        console.error(error)
                        callback(
                            new ApiResponse({
                                status: this.DEFAULT_HTTP_STATUS_CODE,
                                ok: false,
                                body: error,
                            })
                        )
                    })
            },
            (error) => {
                console.error(error)
                callback(
                    new ApiResponse({
                        status: this.DEFAULT_HTTP_STATUS_CODE,
                        ok: false,
                        body: error,
                    })
                )
            }
        )
    }
    post: (url: string, body: any, callback: (data: any) => void) => void = (
        url: string,
        body: any,
        callback: (data: any) => void
    ) => {
        HttpService.post(url, body).then(
            (response: Response) => {
                response
                    .json()
                    .then((json: any) => {
                        let apiResponse = new ApiResponse({ status: response.status, body: json })
                        if (response.ok) {
                            apiResponse.ok = true
                        } else {
                            console.error(response)
                            apiResponse.ok = false
                        }
                        callback(apiResponse)
                    })
                    .catch((error) => {
                        console.error(error)
                        callback(
                            new ApiResponse({
                                status: this.DEFAULT_HTTP_STATUS_CODE,
                                ok: false,
                                body: error,
                            })
                        )
                    })
            },
            (error) => {
                console.error(error)
                callback(
                    new ApiResponse({
                        status: this.DEFAULT_HTTP_STATUS_CODE,
                        ok: false,
                        body: error,
                    })
                )
            }
        )
    }
    postItems: (url: string, body: any, callback: (data: any) => void) => void = (
        url: string,
        body: any,
        callback: (data: any) => void
    ) => {
        HttpService.post(url, body).then(
            (response: Response) => {
                response
                    .json()
                    .then((json: any) => {
                        let apiResponse = new ApiResponse({ status: response.status, body: json })
                        if (response.ok) {
                            apiResponse.ok = true
                        } else {
                            console.error(response)
                            apiResponse.ok = false
                        }
                        callback(apiResponse)
                    })
                    .catch((error) => {
                        console.error(error)
                        callback(
                            new ApiResponse({
                                status: this.DEFAULT_HTTP_STATUS_CODE,
                                ok: false,
                                body: error,
                            })
                        )
                    })
            },
            (error) => {
                console.error(error)
                callback(
                    new ApiResponse({
                        status: this.DEFAULT_HTTP_STATUS_CODE,
                        ok: false,
                        body: error,
                    })
                )
            }
        )
    }
    delete: (url: string, body: any, callback: (data: any) => void) => void = (
        url: string,
        body: any,
        callback: (data: any) => void
    ) => {
        HttpService.delete(url, body).then(
            (response: Response) => {
                response
                    .json()
                    .then((json: any) => {
                        let apiResponse = new ApiResponse({ status: response.status, body: json })
                        if (response.ok) {
                            apiResponse.ok = true
                        } else {
                            console.error(response)
                            apiResponse.ok = false
                        }
                        callback(apiResponse)
                    })
                    .catch((error) => {
                        console.error(error)
                        callback(
                            new ApiResponse({
                                status: this.DEFAULT_HTTP_STATUS_CODE,
                                ok: false,
                                body: error,
                            })
                        )
                    })
            },
            (error) => {
                console.error(error)
                callback(
                    new ApiResponse({
                        status: this.DEFAULT_HTTP_STATUS_CODE,
                        ok: false,
                        body: error,
                    })
                )
            }
        )
    }
}
