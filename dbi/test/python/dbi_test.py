# command tests:
# <req> a collection/doc/field/nested field that does not exist (DNE)
# <req> a collection/doc/field/nested field that does exist
# <req> a collection/doc/field/nested field with bad args

import pytest
from conftest import *
import json
import os

mongo = Mongo()

### HELPER FUNCS

# this assumes that DBI-GET is working, so only use for DEL/SET/POST tests!!!
def check_local_and_remote(uri, result):
    # parse URI and determine how to check result
    frags = uri.split("/")[1:]

    # local check
    local = send("get", "/%s/%s/%s" % (frags[0], frags[1], frags[2])) # retrieve the current doc
    assert result == local # asserts that the local document in dbi matches what we got

    # mongo check
    remote = mongo.grab_document(uri)
    assert remote != False  # mongo returns False if doc DNE
    del remote['_id']       # remove metadata -- isnt tracked locally
    del remote['_version']  # ^^
    assert result == remote  # asserts that the remote document in mongo matches what we got

# helper function that gets passed as an argument to the parametrization of each test function
def generate_new_uris(class_name):
    mongo.add_documents(class_name, pytest.numcollections, pytest.numdocuments, pytest.numlayers)
    mongo.add_auditlogs(class_name, pytest.numdocuments)
    cmds = populate_uris(class_name)
    
    return cmds

# populates command (test params) with JUST a URI (no body -- use only for set and delete) 
def populate_uris(class_name):
    cmds = [] # return value

    # start with standard dbi database
    colls = mongo.client["dbi"].list_collection_names()
    for col in colls:
        if not class_name in col:
            continue # we only want to get collections meant for this testing group
        cmds = ["/dbi/%s" % col] + cmds # prepend
        cursor = mongo.client["dbi"][col].find({})
        for doc in cursor:
            cmds = ["/dbi/%s/%s" % (col, doc["_id"])] + cmds # prepend
            cmds = populate_uris_recursive(doc, "/dbi/%s/%s" % (col, doc["_id"])) + cmds # prepend

    # add URIs for audit log database as well
    cursor = mongo.client["audit"]["log"].find({ "_id": { "$regex": class_name } }) # finds all audit logs that match the current class (method)
    for doc in cursor:
        cmds = ["/dbi/audit/%s" % doc["_id"]] + cmds # prepend
        cmds = populate_uris_recursive(doc, "/dbi/audit/%s" % doc["_id"]) + cmds # prepend

    return cmds

# recursive helper
def populate_uris_recursive(doc, uri):
    cmds = [] # return value
    for field, value in doc.items():
        if field == "_id":
            continue
        cmds = ["%s/%s" % (uri, field)] + cmds # prepend
        if type(value) is dict:
            cmds = populate_uris_recursive(value, "%s/%s" % (uri, field)) + cmds # prepend
    return cmds

# takes a list of uris and generates bodies for POST/SET, returning tuples
def generate_new_params(class_name, new=True):
    uris = generate_new_uris(class_name)

    params = [] # return value - array of (<uri>, <body>)
    
    file = open('args/doc_args.json') # template 2 is similar to 1 but with different values -- original contents of test are based off of template 1
    doc_args = json.load(file)
    file.close()
    doc_args['map'] = doc_args.copy() # add a nested map
    doc_args_str = str(doc_args).replace('True', 'true').replace('False', 'false').replace("'", '"')

    file = open('args/field_args.json') # field_args is template 2 as strings, which is needed for passing field values as args to dbi
    field_args = json.load(file)
    file.close()
    field_args['map'] = str(doc_args['map'].copy()).replace('True', 'true').replace('False', 'false').replace("'", '"') # add a nested map
    
    for uri in uris:
        frags = uri.split("/")[2:]
        if new:
            if len(frags) == 1: # collection
                params.append((uri, "%s/args/collection_args.json" % os.getcwd()))
                params.append(("%s/doc_new" % uri, doc_args_str))
            elif len(frags) == 2: # document
                renamed_body = {}
                for key, value in doc_args.items():
                    renamed_body["%s_new" % key] = value
                renamed_body_str = str(renamed_body).replace('True', 'true').replace('False', 'false').replace("'", '"')
                if frags[0] != "audit": # this is not a valid test case for audit logging
                    params.append(("/dbi/%s_new/%s" % (frags[0], frags[1]), renamed_body_str))
                params.append((uri, renamed_body_str))
            else: # fields
                splits = uri.rsplit("/", 1)
                body = field_args[frags[len(frags)-1]]

                params.append(("%s/map_new/%s" % (splits[0], splits[1]), body))
                params.append(("%s/map_new/%s" % (splits[0], splits[1]), '{"value" : %s }' % body))

                params.append(("%s_new" % uri, body))
                params.append(("%s_new" % uri, '{"value" : %s }' % body))

                if frags[len(frags)-1] == "map":
                    params.append((uri, '{ "stimmt" : true, "etwas" : "quatsch" }'))
        else:
            if len(frags) == 1: # collection
                body = {}
                doc_names  = send("get", "%s/show_documents" % uri)
                for name in doc_names:
                    body[name] = doc_args

                path = tmp_json(body, '%s_test' % frags[0])
                params.append((uri, path))
            elif len(frags) == 2: # document
                params.append((uri, doc_args_str))
            else: # field
                # all body args for field POST/SET need to be strings in order for send to handle them
                field = frags[len(frags)-1]
                body = field_args[field]

                params.append((uri, body)) # straightforward assignment
                params.append((uri, '{"value" : %s }' % body)) # value-map assignment

    return params

# performs recursive field comparison
def check_for_field(new, old, frags):
    field = frags[0]
    assert field in new

    if field not in old:
        del new[field]
        assert old == new

        return

    elif len(frags) == 1: # final frag
        assert old[field] != new[field]
        del old[field]
        del new[field]
        assert old == new

        return

    # recurse
    check_for_field(new[field], old[field], frags[1:])

# performs recursive field comparison
def get_field_layers(new, old, frags):
    field = frags[0]
    assert field in new
    if len(frags) == 1 or field not in old: # final frag, or field is new
        return new, old

    return get_field_layers(new[field], old[field], frags[1:])

### GET TESTS
class TestGet:
    @pytest.mark.parametrize('uri', generate_new_uris("GET"))
    def test_get(self, uri): # the params ^ are inserted as request.param
        response = send("get", uri)

        # parse URI and determine how to check result
        frags = uri.split("/")[2:] # ignores "" and "dbi" from the split

        assert len(frags) > 0

        if len(frags) == 1:
            self.check_collection(frags[0], response)
        elif len(frags) == 2:
            self.check_document(uri, response)
        else:
            doc = mongo.grab_document(uri) # find the document we are working with
            assert doc != False # grab returns False if doc DNE
            self.check_field(frags[2:], response, doc) # frags[2:] prunes the collection and doc names off the frag list
    
    def check_collection(self, collection, result):
        assert type(result) == dict
        for doc_name, doc in result.items():
            self.check_document("/dbi/%s/%s" % (collection, doc_name), doc)

    def check_document(self, uri, result):
        assert type(result) == dict

        doc = mongo.grab_document(uri) # find the document we are working with
        assert doc != False # grab returns False if doc DNE
        del doc['_id']       # remove metadata -- isnt tracked locally

        assert doc == result

    def check_field(self, fieldfrags, result, doc_map): # doc_map is a map (response inside a doc or other map) or doc (both are dicts)
        assert type(doc_map) == dict
        if len(fieldfrags) == 1: # if we are only checking one field endpoint
            assert result == doc_map[fieldfrags[0]]
        else: # else we recurse through the field frags and attempt to find the endpoint
            assert fieldfrags[0] in doc_map # check that we can go to the next layer
            self.check_field(fieldfrags[1:], result, doc_map[fieldfrags[0]]) # prune first entry in field frags, go to next dict layer using key

    # tests show_collections, show_documents, show_map -- combined bc they all rely on the previous being correct
    def test_special(self):
        response = send("get", "/dbi/show_collections")
        colls = mongo.client["dbi"].list_collection_names()

        for coll in colls:
            assert coll in response

            if "DEL" in coll:
                continue # we want to skip any DELETE collections, because their deletion can interrupt our tests

            doc_list = send("get", "/dbi/%s/show_documents" % coll)
            doc_map = send("get", "/dbi/%s/show_map" % coll)
            for doc_name, doc in doc_map.items():
                assert doc_name in doc_list
                remote = mongo.grab_document("/dbi/%s/%s" % (coll, doc_name))
                del remote['_id'] # remove metadata, this will always be here
                try:
                    del remote['_version'] # remove metadata
                except: KeyError # this may be here, depends on how the doc was inserted/updated
                assert remote == doc 

### DELETE TESTS
# note: uses GET to verify results -- make sure GET is working first!
class TestDelete:
    @pytest.mark.parametrize('uri', generate_new_uris("DEL"))
    def test_delete(self, uri):
        response = send("del", uri)

        # parse URI and determine how to check result
        frags = uri.split("/")[2:] # ignores "" and "dbi" from the split
        assert len(frags) > 0
        assert type(response) != string # would only be a string if there was an error

        if len(frags) == 1: # collection
            assert type(response) is list
            assert frags[0] not in response # collection should have been removed from return val

            # remote check
            colls = mongo.client["dbi"].list_collection_names()
            assert frags[0] not in colls

            # local check
            colls = send("get", "/dbi/show_collections")
            assert frags[0] not in colls
        elif len(frags) == 2: # document
            assert type(response) is list
            assert not frags[1] in response # doctument should have been removed from return val

            # remote check
            doc = mongo.grab_document(uri)
            assert not doc # should be False (DNE)

            # local check
            docs = send("get", "/dbi/%s/show_documents" % frags[0])
            assert frags[1] not in docs
        else: # fields
            assert type(response) is dict
            check_local_and_remote(uri, response)

### POST TESTS
# note: uses GET to verify results -- make sure GET is working first!
class TestPost:
    # alters the current uri and existing body fields
    # behavior should be the same as SET in this regard
    @pytest.mark.parametrize('uri, body', generate_new_params("POST_c", False))
    def test_post_change(self, uri, body, save_restore_state):
        frags = uri.split("/")[2:] # ignores "" and "dbi" from the split
        prev_coll_state = save_restore_state

        if len(frags) == 1: # collection
            response = send("set", uri, body, True)

            # check response
            assert response == send("get", "%s" % uri)
            assert type(response) is dict

            # check response
            assert type(response) is dict
            for key, value in prev_coll_state.items():
                assert response[key] != value

            for doc_name, doc in response.items():
                # check local and remote
                check_local_and_remote("/dbi/%s/%s" % (frags[0], doc_name), doc)

                assert doc_name in prev_coll_state # should exist
                assert doc != prev_coll_state[doc_name] # should be different

                # remove from our prev_coll_state
                del prev_coll_state[doc_name]

            # verify that prev_coll_state is now empty
            # all original docs should have been overridden and removed from our dict (see above)
            assert len(prev_coll_state) == 0
        else:
            assert frags[1] in prev_coll_state # doc should exist in coll state
            prev_doc = prev_coll_state[frags[1]]
            response = send("post", uri, body)

            # check document local and remote (will eval fields too)
            check_local_and_remote(uri, response)
            if len(frags) == 2: # document
                assert type(response) is dict
                assert response != prev_doc

                # clear out changed responses from prev_doc
                for field_name in response:
                    assert field_name in prev_doc
                    del prev_doc[field_name]

                # all original fields should have been overridden and removed from our dict (see above)
                assert len(prev_doc) == 0
            else: # field
                field_frags = frags[2:]

                # performs recursive field comparison
                check_for_field(response, prev_doc, field_frags)

    # POSTs a new entry under the uri (+ 1 frag)
    @pytest.mark.parametrize('uri, body', generate_new_params("POST_n"))
    def test_post_new(self, uri, body, save_restore_state):
        frags = uri.split("/")[2:] # ignores "" and "dbi" from the split
        prev_coll_state = save_restore_state

        if len(frags) == 1:
            response = send("post", uri, body, True)

            # check response
            assert type(response) is dict
            assert len(response) != len(prev_coll_state) # something was added

            for doc_name, doc in response.copy().items():
                # check local and remote
                check_local_and_remote("/dbi/%s/%s" % (frags[0], doc_name), doc)
                # remove from our prev_coll_state
                if doc_name not in prev_coll_state:
                    del response[doc_name]

            # verify that response is now the same as prev
            # all new docs should have been overridden and removed from our dict (see above)
            assert response == prev_coll_state
        else:
            response = send("post", uri, body)

            # check document local and remote (will eval fields too)
            check_local_and_remote(uri, response)
            if len(frags) == 2: # document
                assert type(response) is dict
                if "new" in frags[1]: # if we are POSTing a new document entirely
                    assert frags[1] not in prev_coll_state # doc should NOT exist in prev coll state

                    new_coll_state = send("get", "/dbi/%s" % frags[0])
                    del new_coll_state[frags[1]]
                    assert new_coll_state == prev_coll_state # should be the only change
                elif "new" in frags[0]: # we are POSTing to a new *COLLECTION* entirely            
                    assert "not found" in prev_coll_state
                    # this is a pass. check_local_remote has already verified the new content exists. nice!
                    pass
                else: # doc already exists, we are POSTing a new *body* to this doc
                    assert frags[1] in prev_coll_state
                    prev_doc = prev_coll_state[frags[1]]

                    # verify keys are new, remove new keys
                    for key in response.copy().keys():
                        if key not in prev_doc: # new
                            del response[key]

                    # docs should now be identical
                    assert response == prev_doc
            else: # field
                field_frags = frags[2:]
                prev_doc = prev_coll_state[frags[1]]

                # get layer where a change occurred
                new, old = get_field_layers(response, prev_doc, field_frags)

                # special case: merging maps
                if frags[len(frags)-1] == "map" and "new" not in uri:
                    new = new["map"]
                    old = old["map"]

                # remove new keys
                for key in new.copy().keys():
                    if key not in old:
                        del new[key]

                # check that bodies are the same
                assert new == old

### SET TESTS
# note: uses GET to verify results -- make sure GET is working first!
class TestSet:
    # alters the current uri and existing body fields
    # behavior should be the same as POST in this regard
    @pytest.mark.parametrize('uri, body', generate_new_params("SET_c", False))
    def test_set_change(self, uri, body, save_restore_state):
        frags = uri.split("/")[2:] # ignores "" and "dbi" from the split
        prev_coll_state = save_restore_state

        if len(frags) == 1: # collection
            response = send("set", uri, body, True)

            # check response
            assert response == send("get", "%s" % uri)
            assert type(response) is dict

            # check response
            assert type(response) is dict
            for key, value in prev_coll_state.items():
                assert response[key] != value

            for doc_name, doc in response.items():
                # check local and remote
                check_local_and_remote("/dbi/%s/%s" % (frags[0], doc_name), doc)

                assert doc_name in prev_coll_state # should exist
                assert doc != prev_coll_state[doc_name] # should be different

                # remove from our prev_coll_state
                del prev_coll_state[doc_name]

            # verify that prev_coll_state is now empty
            # all original docs should have been overridden and removed from our dict (see above)
            assert len(prev_coll_state) == 0
        else:
            assert frags[1] in prev_coll_state # doc should exist in coll state
            prev_doc = prev_coll_state[frags[1]]
            response = send("set", uri, body)

            # check document local and remote (will eval fields too)
            check_local_and_remote(uri, response)
            if len(frags) == 2: # document
                assert type(response) is dict
                assert response != prev_doc

                # clear out changed responses from prev_doc
                for field_name in response:
                    del prev_doc[field_name]

                # all original fields should have been overridden and removed from our dict (see above)
                assert len(prev_doc) == 0
            else: # field
                field_frags = frags[2:]

                # performs recursive field comparison
                check_for_field(response, prev_doc, field_frags)

    # SETs a new entry under the uri (+ 1 frag)
    @pytest.mark.parametrize('uri, body', generate_new_params("SET_n"))
    def test_set_new(self, uri, body, save_restore_state):
        frags = uri.split("/")[2:] # ignores "" and "dbi" from the split
        prev_coll_state = save_restore_state

        if len(frags) == 1:
            response = send("set", uri, body, True)

            # check response
            assert type(response) is dict

            # key sets should be totally different -- all old docs removed
            for key in response.keys():
                assert key not in prev_coll_state
            for key in prev_coll_state.keys():
                assert key not in response

            for doc_name, doc in response.copy().items():
                # check local and remote
                check_local_and_remote("/dbi/%s/%s" % (frags[0], doc_name), doc)
        else:
            response = send("set", uri, body)

            # check document local and remote (will eval fields too)
            check_local_and_remote(uri, response)
            if len(frags) == 2: # document
                assert type(response) is dict
                if "new" in frags[1]: # if we are SETing a new document entirely
                    # same behavior as POST
                    assert frags[1] not in prev_coll_state # doc should NOT exist in prev coll state

                    new_coll_state = send("get", "/dbi/%s" % frags[0])
                    del new_coll_state[frags[1]]
                    assert new_coll_state == prev_coll_state # should be the only change
                elif "new" in frags[0]: # we are SETing to a new *COLLECTION* entirely
                    # same behavior as POST
                    assert "not found" in prev_coll_state
                    # this is a pass. check_local_remote has already verified the new content exists. nice!
                    pass
                else: # doc already exists, we are SETing a new *body* to this doc
                    assert frags[1] in prev_coll_state
                    prev_doc = prev_coll_state[frags[1]]

                    # key sets should be totally different -- all old fields removed
                    for key in response.keys():
                        assert key not in prev_doc
                    for key in prev_doc.keys():
                        assert key not in response
            else: # field
                field_frags = frags[2:]
                prev_doc = prev_coll_state[frags[1]]

                # get layer where a change occurred
                new, old = get_field_layers(response, prev_doc, field_frags)

                # special case: overriding existing map values
                if frags[len(frags)-1] == "map" and "new" not in uri:
                    new = new["map"]
                    old = old["map"]

                    # key sets should be totally different -- all old fields removed
                    for key in old.keys():
                        assert key not in new
                    for key in new.keys():
                        assert key not in old
                else:
                    assert old != new

                    # prune old keys
                    for key in old.keys():
                        assert key in new
                        del new[key]

                    assert len(new) == 1 # should be just one
                    for key in new.keys(): 
                        assert key in uri

# if you have scrolled this far: hello :)