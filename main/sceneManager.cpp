#include "main/sceneManager.h"

BasicSimulationProperties::BasicSimulationProperties(
    const std::vector<float> &frequencies, float sourcePower,
    int numOfCollectors, int numOfRaysSquared, int maxTracking)
    : frequencies(frequencies), sourcePower(sourcePower),
      numOfCollectors(numOfCollectors), numOfRaysSquared(numOfRaysSquared),
      maxTracking(maxTracking) {

  std::stringstream errorStream;
  if (frequencies.empty()) {
    errorStream
        << "Frequencies in BasicSimulationProperties cannot be empty!\n";
  }
  if (sourcePower < 0) {
    errorStream << "Source Power given cannot be < 0! \n";
  }
  if (numOfCollectors < 4) {
    errorStream << "Number of collectors cannot be less then 4! \n";
  }
  if (numOfCollectors % 4 != 0 && (numOfCollectors - 1) % 4 != 0) {
    errorStream << "number of collectors or number of collectors -1 must be "
                   "divisable by 4! \n";
  }
  if (numOfRaysSquared < 1) {
    errorStream << "numOfRaysSquared in must be greater then 0! \n";
  }
  if (maxTracking < 1) {
    errorStream << "Max tracking in: must be greater then 1 \n";
  }
  std::string outputErrorMessage = errorStream.str();
  if (!outputErrorMessage.empty()) {
    std::stringstream errorInfo;
    errorInfo << "Error detected in: " << *this << "\n" << outputErrorMessage;
    throw std::invalid_argument(errorInfo.str());
  }
}

void BasicSimulationProperties::printItself(std::ostream &os) const noexcept {
  os << "BasicSimulationProperties data class"
     << "\n"
     << "frequencies: ";
  for (const float &freq : frequencies) {
    os << freq << ", ";
  }
  os << "\n"
     << "Source Power: " << sourcePower << "\n"
     << "Number Of Collectors: " << numOfCollectors << "\n"
     << "Number of Rays Squared: " << numOfRaysSquared << "\n";
}

SimulationProperties::SimulationProperties(
    collectionRules::CollectEnergyInterface *energyCollectionRules,
    const BasicSimulationProperties &basicSimulationProperties)
    : energyCollectionRules_(energyCollectionRules),
      basicSimulationProperties_(basicSimulationProperties){};

void SimulationProperties::printItself(std::ostream &os) const noexcept {
  os << "SimulationProperties\n"
     << "- Energy collection rules: \n"
     << *(energyCollectionRules_) << "- Basic Simulation Properties: \n"
     << basicSimulationProperties_;
}

SceneManager::SceneManager(
    Model *model, const SimulationProperties &simulationProperties,
    trackers::PositionTrackerInterface *positionTracker,
    trackers::CollectorsTrackerInterface *collectorTracker)
    : model_(model), simulationProperties_(simulationProperties),
      raytracer_(model), positionTracker_(positionTracker),
      collectorsTracker_(collectorTracker) {
  offseter_ = std::make_unique<generators::FakeOffseter>();
}

void SceneManager::printItself(std::ostream &os) const noexcept {
  os << "SCENE MANAGER\n"
     << "Model: " << *(model_) << "\n"
     << "Simulation properties: " << simulationProperties_ << "\n"
     << "Ray Tracer: " << raytracer_ << "\n"
     << "Position Tracker: " << *(positionTracker_) << "\n"
     << "Collectors Tracker: " << *(collectorsTracker_) << "\n"
     << "Offseter: " << *(offseter_);
}

std::unordered_map<float, Collectors> SceneManager::run() {
  std::vector<float> frequencies =
      simulationProperties_.basicSimulationProperties().frequencies;

  std::unordered_map<float, Collectors> collectorsPerFrequencies;

  for (float freq : frequencies) {

    // Initialize frequency in visual reporesentation of the simulation
    positionTracker_->initializeNewFrequency(freq);

    generators::PointSpeakerRayFactory pointSpeaker(
        simulationProperties_.basicSimulationProperties().numOfRaysSquared,
        simulationProperties_.basicSimulationProperties().sourcePower, model_);

    Simulator simulator(&raytracer_, model_, &pointSpeaker, offseter_.get(),
                        positionTracker_,
                        simulationProperties_.energyCollectionRules());

    Collectors collectors = buildCollectors(
        model_,
        simulationProperties_.basicSimulationProperties().numOfCollectors);

    // Save collectors for the visual representation
    collectorsTracker_->save(collectors, "./data");

    int maxTracking =
        simulationProperties_.basicSimulationProperties().maxTracking;
    simulator.run(freq, &collectors, maxTracking);

    collectorsPerFrequencies.insert(
        std::make_pair(freq, std::move(collectors)));
    positionTracker_->endCurrentFrequency();
  }
  positionTracker_->save();
  return collectorsPerFrequencies;
}
