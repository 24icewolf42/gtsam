import gtsam
from gtsam.examples.SFMdata import *
from gtsam.utils import *
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import time # for sleep()

def visual_ISAM2_plot(poses, points, result):
    # VisualISAMPlot plots current state of ISAM2 object
    # Author: Ellon Paiva
    # Based on MATLAB version by: Duy Nguyen Ta and Frank Dellaert

    # Declare an id for the figure
    fignum = 0;

    fig = plt.figure(fignum)
    ax = fig.gca(projection='3d')
    plt.cla()

    # Plot points
    # Can't use data because current frame might not see all points
    # marginals = Marginals(isam.getFactorsUnsafe(), isam.calculateEstimate()); # TODO - this is slow
    # gtsam.plot3DPoints(result, [], marginals);
    plot3DPoints(fignum, result, 'rx');

    # Plot cameras
    M = 0;
    while result.exists(int(gtsam.Symbol('x',M))):
        ii = int(gtsam.Symbol('x',M));
        pose_i = result.pose3_at(ii);
        plotPose3(fignum, pose_i, 10);
        
        M = M + 1;

    # draw
    ax.set_xlim3d(-40, 40)
    ax.set_ylim3d(-40, 40)
    ax.set_zlim3d(-40, 40)
    plt.show(block=False)
    plt.draw();

def visual_ISAM2_example():
    # Define the camera calibration parameters
    K = gtsam.Cal3_S2(50.0, 50.0, 0.0, 50.0, 50.0)

    # Define the camera observation noise model
    measurementNoise = gtsam.noiseModel.Isotropic.Sigma(2, 1.0) # one pixel in u and v

    # Create the set of ground-truth landmarks
    points = createPoints() # from SFMdata

    # Create the set of ground-truth poses
    poses = createPoses() # from SFMdata

    # Create an iSAM2 object. Unlike iSAM1, which performs periodic batch steps to maintain proper linearization
    # and efficient variable ordering, iSAM2 performs partial relinearization/reordering at each step. A parameter
    # structure is available that allows the user to set various properties, such as the relinearization threshold
    # and type of linear solver. For this example, we we set the relinearization threshold small so the iSAM2 result
    # will approach the batch result.
    parameters = gtsam.ISAM2Params()
    parameters.relinearize_threshold = 0.01
    parameters.relinearize_skip = 1
    isam = gtsam.ISAM2(parameters)

    # Create a Factor Graph and Values to hold the new data
    graph = gtsam.NonlinearFactorGraph()
    initialEstimate = gtsam.Values()

    #  Loop over the different poses, adding the observations to iSAM incrementally
    for i, pose in enumerate(poses):

        # Add factors for each landmark observation
        for j, point in enumerate(points):
            camera = gtsam.PinholeCameraCal3_S2(pose, K)
            measurement = camera.project(point)
            graph.push_back(gtsam.GenericProjectionFactorCal3_S2(measurement, measurementNoise, int(gtsam.Symbol('x', i)), int(gtsam.Symbol('l', j)), K))

        # Add an initial guess for the current pose
        # Intentionally initialize the variables off from the ground truth
        initialEstimate.insert(int(gtsam.Symbol('x', i)), pose.compose(gtsam.Pose3(gtsam.Rot3.Rodrigues(-0.1, 0.2, 0.25), gtsam.Point3(0.05, -0.10, 0.20))))

        # If this is the first iteration, add a prior on the first pose to set the coordinate frame
        # and a prior on the first landmark to set the scale
        # Also, as iSAM solves incrementally, we must wait until each is observed at least twice before
        # adding it to iSAM.
        if( i == 0):
            # Add a prior on pose x0
            poseNoise = gtsam.noiseModel.Diagonal.Sigmas(np.array([0.3, 0.3, 0.3, 0.1, 0.1, 0.1])) # 30cm std on x,y,z 0.1 rad on roll,pitch,yaw
            graph.push_back(gtsam.PriorFactorPose3(int(gtsam.Symbol('x', 0)), poses[0], poseNoise))

            # Add a prior on landmark l0
            pointNoise = gtsam.noiseModel.Isotropic.Sigma(3, 0.1)
            graph.push_back(gtsam.PriorFactorPoint3(int(gtsam.Symbol('l', 0)), points[0], pointNoise)) # add directly to graph
            
            # Add initial guesses to all observed landmarks
            # Intentionally initialize the variables off from the ground truth
            for j, point in enumerate(points):
                initialEstimate.insert(int(gtsam.Symbol('l', j)), point + gtsam.Point3(-0.25, 0.20, 0.15));
        else:
            # Update iSAM with the new factors
            isam.update(graph, initialEstimate)
            # Each call to iSAM2 update(*) performs one iteration of the iterative nonlinear solver.
            # If accuracy is desired at the expense of time, update(*) can be called additional times
            # to perform multiple optimizer iterations every step.
            isam.update()
            currentEstimate = isam.calculate_estimate();
            print "****************************************************"
            print "Frame", i, ":"
            for j in range(i+1):
                print gtsam.Symbol('x',j)
                print currentEstimate.pose3_at(int(gtsam.Symbol('x',j)))

            for j in range(len(points)):
                print gtsam.Symbol('l',j)
                print currentEstimate.point3_at(int(gtsam.Symbol('l',j)))

            visual_ISAM2_plot(poses, points, currentEstimate);
            time.sleep(1)

            # Clear the factor graph and values for the next iteration
            graph.resize(0);
            initialEstimate.clear();

if __name__ == '__main__':
    visual_ISAM2_example()
