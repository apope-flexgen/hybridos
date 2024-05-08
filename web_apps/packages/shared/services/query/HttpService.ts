let instance: HttpService

class HttpService {
  constructor() {
    if (!instance) {
      instance = this
    }

    return instance
  }
  get: (string) => Promise<Response> = (url: string) => {
    try {
      return fetch(url, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json',
        },
        credentials: 'include',
      })
    } catch (error) {
      console.error(error)
      return Promise.reject(error)
    }
  }
  post: (string, any) => Promise<Response> = (url: string, body: any) => {
    try {
      return fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(body),
        credentials: 'include',
      })
    } catch (error) {
      console.error(error)
      return Promise.reject(error)
    }
  }
  delete: (string, any) => Promise<Response> = (url: string, body: any) => {
    try {
      return fetch(url, {
        method: 'DELETE',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(body),
        credentials: 'include',
      })
    } catch (error) {
      console.error(error)
      return Promise.reject(error)
    }
  }
}

export default new HttpService()
