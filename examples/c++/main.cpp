/// \ingroup examples
/// \author Julien Tierny <julien.tierny@lip6.fr>
/// \date October 2017.
/// 
/// \brief Minimalist C++-only TTK example pipeline, including:
///  -# The computation of a persistence curve
///  -# The computation of a persistence diagram
///  -# The selection of the most persistent pairs of the diagram
///  -# The pre-simplification of the data according to this selection
///  -# The computation of the Morse-Smale complex on this simplified data
///  -# The storage of the output of this pipeline to disk.
///
/// This reproduces the Figure 1 of the TTK companion paper:
/// "The Topology ToolKit", J. Tierny, G. Favelier, J. Levine, C. Gueunet, M.
/// Michaux., IEEE Transactions on Visualization and Computer Graphics, Proc.
/// of IEEE VIS 2017.
///
/// See the individual VTK wrappers to see how to use each ttk::base (C++-only)
/// TTK component.

// include the local headers
#include                  <Blank.h>
#include                  <MorseSmaleComplex.h>
#include                  <PersistenceCurve.h>
#include                  <PersistenceDiagram.h>
#include                  <ProgramBase.h>
#include                  <TopologicalSimplification.h>

template <class ttkModule>
  class ExampleProgram : public Program<ttkModule>{
  
  public:
    
    int execute(){
      
      // computing some elevation
      vector<float> height(pointSet_.size()/3);
      vector<int> offsets(height.size());
      int vertexId = 0;
      // use the z-coordinate here
      for(int i = 2; i < (int) pointSet_.size(); i+=3){
        height[vertexId] = pointSet_[i];
        offsets[vertexId] = vertexId;
        vertexId++;
      }
      
      // 2. computing the persistence curve
      ttk::PersistenceCurve curve;
      vector<pair<float, ftm::idVertex> > outputCurve;
      curve.setupTriangulation(&triangulation_);
      curve.setInputScalars(height.data());
      curve.setInputOffsets(offsets.data());
      curve.setOutputCTPlot(&outputCurve);
      curve.execute<float>();
      
      // 3. computing the persitence diagram
      ttk::PersistenceDiagram diagram;
      vector<tuple<ftm::idVertex, ftm::NodeType, 
        ftm::idVertex, ftm::NodeType, float, ftm::idVertex> >
        diagramOutput;
      diagram.setupTriangulation(&triangulation_);
      diagram.setInputScalars(height.data());
      diagram.setInputOffsets(offsets.data());
      diagram.setOutputCTDiagram(&diagramOutput);
      diagram.execute<float>();
      
      // 4. selecting the critical point pairs
      vector<float> simplifiedHeight = height;
      vector<int> authorizedCriticalPoints, simplifiedOffsets = offsets;
      for(int i = 0; i < (int) diagramOutput.size(); i++){
        double persistence = get<4>(diagramOutput[i]);
        if(persistence > 0.2){
          // 5. selecting the most persistent pairs
          authorizedCriticalPoints.push_back(get<0>(diagramOutput[i]));
          authorizedCriticalPoints.push_back(get<2>(diagramOutput[i]));
        }
      }
     
      // 6. simplifying the input data to remove non-persistent pairs
      ttk::TopologicalSimplification simplification;
      simplification.setupTriangulation(&triangulation_);
      simplification.setInputScalarFieldPointer(height.data());
      simplification.setInputOffsetScalarFieldPointer(offsets.data());
      simplification.setOutputOffsetScalarFieldPointer(
        simplifiedOffsets.data());
      simplification.setOutputScalarFieldPointer(simplifiedHeight.data());
      simplification.setConstraintNumber(authorizedCriticalPoints.size());
      simplification.setVertexIdentifierScalarFieldPointer(
        authorizedCriticalPoints.data());
      simplification.execute<float>();
      
      // assign the simplified values to the input mesh
      for(int i = 0; i < (int) simplifiedHeight.size(); i++){
        pointSet_[3*i + 2] = simplifiedHeight[i];
      }
      
      // 7. computing the Morse-Smale complex
      ttk::MorseSmaleComplex morseSmaleComplex;
      // critical points
      int criticalPoints_numberOfPoints;
      vector<float> criticalPoints_points;
      vector<int> criticalPoints_points_cellDimensions;
      vector<int> criticalPoints_points_cellIds;
      vector<char> criticalPoints_points_isOnBoundary;
      vector<float> criticalPoints_points_cellScalars;
      vector<int> criticalPoints_points_PLVertexIdentifiers;
      vector<int> criticalPoints_points_manifoldSize;
      // 1-separatrices
      int separatrices1_numberOfPoints;
      vector<float> separatrices1_points;
      vector<char> separatrices1_points_smoothingMask;
      vector<int> separatrices1_points_cellDimensions;
      vector<int> separatrices1_points_cellIds;
      int separatrices1_numberOfCells{};
      vector<int> separatrices1_cells;
      vector<int> separatrices1_cells_sourceIds;
      vector<int> separatrices1_cells_destinationIds;
      vector<int> separatrices1_cells_separatrixIds;
      vector<char> separatrices1_cells_separatrixTypes;
      vector<char> separatrices1_cells_isOnBoundary;
      vector<float> separatrices1_cells_separatrixFunctionMaxima;
      vector<float> separatrices1_cells_separatrixFunctionMinima;
      vector<float> separatrices1_cells_separatrixFunctionDiffs;
      // segmentation 
      vector<int> 
        ascendingSegmentation(triangulation_.getNumberOfVertices(), -1),
        descendingSegmentation(triangulation_.getNumberOfVertices(), -1), 
        mscSegmentation(triangulation_.getNumberOfVertices(), -1);
      morseSmaleComplex.setupTriangulation(&triangulation_);
      morseSmaleComplex.setInputScalarField(simplifiedHeight.data());
      morseSmaleComplex.setInputOffsets(simplifiedOffsets.data());
      morseSmaleComplex.setOutputMorseComplexes(
        ascendingSegmentation.data(),
        descendingSegmentation.data(),
        mscSegmentation.data());
      morseSmaleComplex.setOutputCriticalPoints(
        &criticalPoints_numberOfPoints,
        &criticalPoints_points,
        &criticalPoints_points_cellDimensions,
        &criticalPoints_points_cellIds,
        &criticalPoints_points_cellScalars,
        &criticalPoints_points_isOnBoundary,
        &criticalPoints_points_PLVertexIdentifiers,
        &criticalPoints_points_manifoldSize);
      morseSmaleComplex.setOutputSeparatrices1(
        &separatrices1_numberOfPoints,
        &separatrices1_points,
        &separatrices1_points_smoothingMask,
        &separatrices1_points_cellDimensions,
        &separatrices1_points_cellIds,
        &separatrices1_numberOfCells,
        &separatrices1_cells,
        &separatrices1_cells_sourceIds,
        &separatrices1_cells_destinationIds,
        &separatrices1_cells_separatrixIds,
        &separatrices1_cells_separatrixTypes,
        &separatrices1_cells_separatrixFunctionMaxima,
        &separatrices1_cells_separatrixFunctionMinima,
        &separatrices1_cells_separatrixFunctionDiffs,
        &separatrices1_cells_isOnBoundary);

      morseSmaleComplex.execute<float>();
      
      return 0;
    }
    
    int load(const vector<string> &inputPaths){
      
      if(inputPaths.empty())
        return -1;
      if(!inputPaths[0].length())
        return -2;
      
      {
        stringstream msg;
        msg << "[ExampleProgram] Reading input mesh..." << endl;
        // choose where to display this message (cout, cerr, a file)
        // choose the priority of this message (1, nearly always displayed, 
        // higher values mean lower priorities)
        Debug::dMsg(cout, msg.str(), Debug::timeMsg);
      }
      
      int vertexNumber = 0, triangleNumber = 0;
      string keyword;
      
      ifstream f(inputPaths[0].data(), ios::in);
      
      if(!f){
        stringstream msg;
        msg << "[Editor] Cannot open file `" 
          << inputPaths[0] << "'!" << endl;
        Debug::dMsg(cerr, msg.str(), Debug::fatalMsg);
        return -1;
      }
      
      f >> keyword;
      
      if(keyword != "OFF"){
        stringstream msg;
        msg << "[Editor] Input OFF file `" 
          << inputPaths[0] << "' seems invalid :(" 
          << endl;
        Debug::dMsg(cerr, msg.str(), Debug::fatalMsg);
        return -2;
      }
      
      f >> vertexNumber;
      f >> triangleNumber;
      f >> keyword;
      
      pointSet_.resize(3*vertexNumber);
      triangleSet_.resize(4*triangleNumber);
      
      for(int i = 0; i < 3*vertexNumber; i++){
        f >> pointSet_[i];
      }
      
      for(int i = 0; i < 4*triangleNumber; i++){
        f >> triangleSet_[i];
      }
      
      f.close();
      
      Blank *smoother = 
        (Blank *) Program<ttkModule>::ttkModule_;
      
      triangulation_.setInputPoints(vertexNumber, pointSet_.data());
      triangulation_.setInputCells(triangleNumber, triangleSet_.data());
      smoother->setupTriangulation(&triangulation_);
      
      {
        stringstream msg;
        msg << "[Editor]   done! (read " 
          << vertexNumber
          << " vertices, "
          << triangleNumber
          << " triangles)" << endl;
        Debug::dMsg(cout, msg.str(), Debug::timeMsg);
      }
      
      return 0;
    }
    
    int save() const{
      
      // save the simplified terrain
        
      string fileName(Program<ttkModule>::outputPath_ + ".off");
      
      ofstream f(fileName.data(), ios::out);
      
      if(!f){
        stringstream msg;
        msg 
          << "[Editor] Could not write output file `" 
          << fileName << "'!" << endl;
        Debug::dMsg(cerr, msg.str(), Debug::fatalMsg);
        return -1;
      }
      
      f << "OFF" << endl;
      f << pointSet_.size()/3 << " " << triangleSet_.size()/4 << " 0" << endl;
      
      for(int i = 0; i < (int) pointSet_.size()/3; i++){
        for(int j = 0; j < 3; j++){
          f << pointSet_[3*i + j];
          f << " ";
        }
        f << endl;
      }
      
      for(int i = 0; i < (int) triangleSet_.size()/4; i++){
        for(int j = 0; j < 4; j++){
          f << triangleSet_[4*i + j];
          f << " ";
        }
        f << endl;
      }
      
      f.close();
      
      return 0;
    }
    
  protected:
    
    vector<float>           pointSet_;
    vector<long long int >  triangleSet_;
    Triangulation           triangulation_;
};

int main(int argc, char **argv) {

  ExampleProgram<Blank> program;
  
  // register the arguments to the command line parser
  string inputFilePath;
  program.parser_.setArgument("i", &inputFilePath, "Path to input OFF file");
  
  int ret = 0;
  ret = program.init(argc, argv);
 
  if(ret != 0)
    return ret;

  // execute data processing
  ret = program.run();
  
  if(ret != 0)
    return ret;
 
  // save the output
  ret = program.save();
  
  return ret;
}
