import sys
import numpy as np
import matplotlib.pyplot as plt
import os
import re
from PIL import Image
import argparse
import pickle
import random

import tensorflow as tf
from tensorflow import keras
from tensorflow.keras.layers import Concatenate, UpSampling2D, Add
from tensorflow.keras.layers import Convolution2D, Dense, MaxPooling2D, Flatten, BatchNormalization, Dropout
from tensorflow.keras import Input, Model

def getData(imageDir, labelDir, filt):
    ins = []
    outs = []

    #extracts all of the files that match the filter
    filt = '*'
    files = [f for f in os.listdir(imageDir) if re.match(r'.%s.jpg'%filt, f)]
    
    #Adds the image to the ins and outs arrays
    for f in files:
        imFile = imageDir + '/' + f
        labFile = labelDir + '/' + f
        
        im = Image.open(imFile)
        ins.append(np.asarray(im))
        
        label = Image.open(labFile)
        label = np.asarray(label)
        outs.append(np.where(label > 100, 1, 0))
    
    #Shapes ins into a 4d numpy array
    sh = ins[0].shape
    newSh = (1, sh[0], sh[1], sh[2])
    ins = [np.reshape(i, newshape = newSh) for i in ins]
    ins = np.concatenate(ins, axis=0)

    #shapes outs into a 3d numpy array
    sh = outs[0].shape
    newSh = (1, sh[0], sh[1])
    outs = [np.reshape(o, newshape = newSh) for o in outs]
    outs = np.concatenate(outs, axis=0)

    return ins, outs


def createModel(inputShape, convLayers):
    inputTensor = Input(shape=inputShape, name="input")

    tensors = [] #used to hold the previous tensors for concatenation
    previousTensor = inputTensor #stores the last tensor
    count = 0
    for i in convLayers:
        name = 'conv%d'%(count+1)
        
        #If not upscaling adds a normal convolution layer
        if i['strides'] > 0:
            tensors.append(Convolution2D(
                        filters = i['filters'],
                        kernel_size = i['kernel'],
                        strides = i['strides'],
                        padding = 'same',
                        activation = 'elu',
                        name = name)(previousTensor))
            previousTensor = tensors[len(tensors) - 1]
            
            #adds a concatenation layer if specified
            if i['concat'] >= 0:
                previousTensor = Concatenate()([previousTensor, tensors[i['concat']]])
            
        #Upsampling. First adds a convolution layer to get the filters correct, then adds the concat if specified and then up samples
        else:
            tensors.append(Convolution2D(
                    filters = i['filters'],
                    kernel_size = 1,
                    strides = 1,
                    padding = 'same',
                    activation = 'elu',
                    name = name)(previousTensor))
            previousTensor = tensors[len(tensors) - 1]
            
            if i['concat'] >= 0:
                previousTensor = Concatenate()([previousTensor, tensors[i['concat']]])
            
            previousTensor = UpSampling2D(size = -1 * i['strides'])(previousTensor)
        
        #Adds a batch normalization layer
        #name = 'batch%d'%(count + 1)
        #previousTensor = BatchNormalization()(previousTensor)
        
        count += 1
        
    #output tensor
    outputTensor = Convolution2D(2,
                                kernel_size = (1,1),
                                padding='same',
                                name = 'output',
                                activation='softmax')(previousTensor)
    
    #creates the model
    model = Model(inputs = inputTensor, outputs = outputTensor)
    
    #creates the optimizer
    opt = tf.keras.optimizers.Adam(lr=.0001, beta_1=.9, beta_2=.999, epsilon=None, decay=0.0, amsgrad=False)
    
    #compiles, prints and returns the model
    model.compile(loss=tf.keras.losses.SparseCategoricalCrossentropy(), optimizer=opt,
                 metrics=[tf.keras.metrics.SparseCategoricalAccuracy()])
    print(model.summary())
    return model

def training_set_generator_images(ins, outs, batch_size=10, input_name='input', output_name='output'):
    while True:
        # Randomly select a set of example indices
        example_indices = random.choices(range(ins.shape[0]), k=batch_size)
        
        # The generator will produce a pair of return values: one for inputs and one for outputs
        yield({input_name: ins[example_indices,:,:,:]},
             {output_name: outs[example_indices,:,:]})

#Generates a file name based on the given model parameters
def generateFileName(args):
    kernelStr='_'.join(str(x) for x in args.kernelSize)
    filterStr = '_'.join(str(x) for x in args.filterSize)
    stridesStr = '_'.join(str(x) for x in args.strides)
        
    return '%s/kernel%s_filter%s_stride%s_epoch%d_pat%d_batch%d'%(args.resultsPath, kernelStr, filterStr, 
                                                                         stridesStr, 
                                                                         args.epochs, 
                                                                         args.patience,
                                                                         args.batchSize)

def execute(args):
    imDir = 'images'
    labelDir = 'labels'

    #TODO set up validation and maybe a bit of test data
    ins, outs = getData(imDir, labelDir, '*')

    #prepares the commandline data for the model
    convLayers = [{'filters': f, 'kernel': k, 'strides': s, 'concat': c}
                 for f, k, s, c in zip(args.filterSize, args.kernelSize, args.strides, args.concat)]

    #Creates the early stopping callback
    early_stopping_cb = keras.callbacks.EarlyStopping(patience=args.patience,
                                                      monitor='sparse_categorical_accuracy',
                                                      restore_best_weights=True,
                                                      min_delta=args.minDelta)

    #Creates the generator
    generator = training_set_generator_images(ins, outs, batch_size=args.batchSize)

    #creates the model
    model = createModel(ins.shape[1:4], convLayers = convLayers)

    #Trains the model
    history = model.fit(x=generator, epochs=args.epochs, steps_per_epoch=args.stepsSize,
                        verbose=args.verbose>=2,
                        #validation_data=(validIns, validOuts), 
                        callbacks=[early_stopping_cb])
    
    '''
    # Generate log data
    results = {}
    results['args'] = args
    
    results['predict_validation'] = model.predict(validIns)
    results['predict_validation_eval'] = model.evaluate(validIns, validOuts)
    results['true_validation'] = validOuts
    results['predict_testing'] = model.predict(testIns)
    results['predict_testing_eval'] = model.evaluate(testIns, testOuts)
    results['true_testing'] = testOuts
    #results['folds'] = folds
    results['history'] = history.history
    '''
    # Save results
    fbase = generateFileName(args)
    #results['fname_base'] = fbase
    #fp = open("%s_results.pkl"%(fbase), "wb")
    #pickle.dump(results, fp)
    #fp.close()
    
    # Model
    model.save("%s_model"%(fbase))
    
    return model                                                    


def createParser():
    parser = argparse.ArgumentParser(description='Semantic Labeling', fromfile_prefix_chars='@')
    parser.add_argument('-epochs', type=int, default=5, help='Number of epochs')
    parser.add_argument('-verbose', '-v', action='count', default=0, help='Verbosity Level')
    parser.add_argument('-patience', type=int, default = 10, help="Patience used for Early Stopping")
    parser.add_argument('-minDelta', type=float, default=.001, help='Minimum delta for early termination')
    parser.add_argument('-resultsPath', type=str, default='results', help='Path to the folder the results should be stored in')
    parser.add_argument('-batchSize', type=int, default=10, help='Batch for keras to break the ins into for each epoch')
    parser.add_argument('-stepsSize', type=int, default=2, help='Steps per epoch')
    parser.add_argument('-kernelSize', nargs='+', type=int, default=[5,3,2], help='Convolution kernel size per layer (sequence of ints)')    
    parser.add_argument('-filterSize', nargs='+', type=int, default=[10, 1, 5], help='Convolution filter size per layer (sequence of ints)')
    parser.add_argument('-strides', nargs='+', type=int, default=[4,1,-4], help='Convolution stride size per layer (sequence of ints)')
    parser.add_argument('-concat', nargs='+', type=int, default=[-1,-1,0], help='Concatenates this layer with the given layer. -1 if no concatenation is wanted')

    parser.add_argument('-trainSet', type=str, default='-*', help='regular expression for the training data')
    parser.add_argument('-validSet', type=str, default='-*[12345]9', help='regular expression for the validation data')
    parser.add_argument('-testSet', type=str, default='-*', help='regular expression for the test data')
    
    return parser
    
if __name__ == "__main__":
    tf.config.threading.set_intra_op_parallelism_threads(12)
    parser = createParser()
    args = parser.parse_args()
    execute(args)