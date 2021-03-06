/*
 * Copyright (c) 2014, Brookhaven Science Associates, Brookhaven        
 * National Laboratory. All rights reserved.                            
 *                                                                      
 * Redistribution and use in source and binary forms, with or without   
 * modification, are permitted provided that the following conditions   
 * are met:                                                             
 *                                                                      
 * * Redistributions of source code must retain the above copyright     
 *   notice, this list of conditions and the following disclaimer.      
 *                                                                      
 * * Redistributions in binary form must reproduce the above copyright  
 *   notice this list of conditions and the following disclaimer in     
 *   the documentation and/or other materials provided with the         
 *   distribution.                                                      
 *                                                                      
 * * Neither the name of the Brookhaven Science Associates, Brookhaven  
 *   National Laboratory nor the names of its contributors may be used  
 *   to endorse or promote products derived from this software without  
 *   specific prior written permission.                                 
 *                                                                      
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS  
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT    
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS    
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE       
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,           
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES   
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR   
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)   
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,  
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OTHERWISE) ARISING   
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   
 * POSSIBILITY OF SUCH DAMAGE.                                          
 *
 */

#include <stdio.h>
#include <Python.h>

/* Include python and numpy header files */
#include <math.h>
#define NPY_NO_DEPRECATED_API NPY_1_9_API_VERSION
#include <numpy/ndarraytypes.h>
#include <numpy/ndarrayobject.h>

#include "image.h"

static PyObject* image_rotate90(PyObject *self, PyObject *args){
  PyObject *_input = NULL;
  PyArrayObject *input = NULL;
  PyArrayObject *out = NULL;
  npy_intp *dims;
  npy_intp temp;
  int ndims, sense = 0;

  if(!PyArg_ParseTuple(args, "Oi", &_input, &sense)){
    return NULL;
  }

  input = (PyArrayObject*)PyArray_FROMANY(_input, NPY_FLOAT, 3, 0,NPY_ARRAY_IN_ARRAY);
  if(!input){
    goto error;
  }

  ndims = PyArray_NDIM(input);
  dims = PyArray_DIMS(input);

  // As we rotate ... swap last 2 dims
  temp = dims[ndims-2];
  dims[ndims-2] = dims[ndims-1];
  dims[ndims-1] = temp;

  out = (PyArrayObject*)PyArray_SimpleNew(ndims, dims, NPY_FLOAT);
  if(!out){
    goto error;
  }
  
  // Swap Dims Back ...
  temp = dims[ndims-2];
  dims[ndims-2] = dims[ndims-1];
  dims[ndims-1] = temp;

  rotate90((data_t*)PyArray_DATA(input), (data_t*)PyArray_DATA(out),
           ndims, dims, sense);

  Py_XDECREF(input);
  return Py_BuildValue("N", out);

error:
  Py_XDECREF(input);
  Py_XDECREF(out);
  return NULL;
}

static PyMethodDef imageMethods[] = {
  { "rotate90", image_rotate90, METH_VARARGS,
    "Rotate stack of images 90 degrees (with sense)"},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef imagemodule = {
   PyModuleDef_HEAD_INIT,
   "image",     /* name of module */
   NULL,        /* module documentation, may be NULL */
   -1,          /* size of per-interpreter state of the module,
                   or -1 if the module keeps state in global variables. */
   imageMethods
};

PyMODINIT_FUNC PyInit_image(void) {
  PyObject *m;
  m = PyModule_Create(&imagemodule);
  if(m == NULL){
    return NULL;
  }

  import_array();

  return m;
}
