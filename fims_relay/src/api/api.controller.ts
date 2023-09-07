import { Controller, Get, Put, Query, Post, Delete, Body } from '@nestjs/common';
import { ApiCreatedResponse, ApiOkResponse, ApiOperation } from '@nestjs/swagger';
import { DELETE_SUMMARY, GET_SUMMARY, POST_SUMMARY, SET_SUMMARY } from './api.constants';
import { ApiService } from './api.service';
import { BodyDTO } from './dto/body.dto';
import { GetDTO } from './dto/get.dto';
import { FimsMsg } from './interface/fimsmsg.interface';
import { FimsMsgResponse } from './responses/fims.response';

@Controller('fims')
export class ApiController {
	constructor(
		private readonly apiService: ApiService
  ) {}

  @Get('get')
  @ApiOperation({summary: GET_SUMMARY})
  @ApiOkResponse({type: FimsMsgResponse})
  async get(@Query() query: GetDTO): Promise<FimsMsgResponse> {
    console.log("GET: ", query);
    const msg: FimsMsg = {
      method: 'get',
      uri: query.uri,
      replyto: query.replyto,
      body: '',
      username: '',
    };
    return await this.apiService.send(msg);
  }

  @Put('set')
  @ApiOperation({summary: SET_SUMMARY})
  @ApiOkResponse({type: FimsMsgResponse})
  async set(@Body() body: BodyDTO): Promise<FimsMsgResponse> {
    console.log("SET: ", body);
    const msg: FimsMsg = {
      method: 'set',
      uri: body.uri,
      replyto: body.replyto,
      body: body.body,
      username: '',
    };
    return await this.apiService.send(msg);
  }

  @Post('post')
  @ApiOperation({summary: POST_SUMMARY})
  @ApiCreatedResponse({type: FimsMsgResponse})
  async post(@Body() body: BodyDTO): Promise<FimsMsgResponse> {
    console.log("POST: ", body);
    const msg: FimsMsg = {
      method: 'post',
      uri: body.uri,
      replyto: body.replyto,
      body: body.body,
      username: '',
    };
    return await this.apiService.send(msg);
  }

  @Delete('delete')
  @ApiOperation({summary: DELETE_SUMMARY})
  @ApiOkResponse({type: FimsMsgResponse})
  async delete(@Body() body: BodyDTO): Promise<FimsMsgResponse> {
    console.log("DELETE: ", body);
    const msg: FimsMsg = {
      method: 'del',
      uri: body.uri,
      replyto: body.replyto,
      body: body.body,
      username: '',
    };
    return await this.apiService.send(msg);
  }
}
