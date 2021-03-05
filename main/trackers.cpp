#include "trackers.h"

namespace trackers {

void FileBuffer::addMessageToBuffer(std::string_view message) {
  stream << message;
}

void FileBuffer::acquireJsonFile(const Json &json) {
  stream.clear();
  stream << json;
}

void FileInterface::printItself(std::ostream &os) const noexcept {
  os << "File Interface class \n";
}

void File::openFileWithOverwrite() {
  fileStream_.open(path_.data());
  handleErrors();
}

void File::open() {
  fileStream_.open(path_.data(), std::ios_base::app);
  handleErrors();
}

void File::write(const FileBuffer &buffer) {
  fileStream_ << buffer.stream.rdbuf() << std::endl;
}

void File::writeWithoutFlush(const FileBuffer &buffer) {
  fileStream_ << buffer.stream.rdbuf();
}

void File::printItself(std::ostream &os) const noexcept {
  os << "File at given path: " << path_ << "\n";
}

void File::handleErrors() {
  if (!fileStream_.good()) {
    std::stringstream errorStream;
    errorStream << "Error in: " << *this << "File doesn't exist at given path!";
    throw std::invalid_argument(errorStream.str());
  }
}

namespace javascript {

FileBuffer initVar(std::string_view variableName) {
  FileBuffer buffer;
  buffer.stream << "var " << variableName << "= ";
  return buffer;
}

FileBuffer initLet(std::string_view variableName) {
  FileBuffer buffer;
  buffer.stream << "let " << variableName << "= ";
  return buffer;
}

FileBuffer initConst(std::string_view variableName) {
  FileBuffer buffer;
  buffer.stream << "const " << variableName << "= ";
  return buffer;
}

FileBuffer initArray() {
  FileBuffer buffer;
  initArrayInBuffer(buffer);
  return buffer;
}

FileBuffer endArray() {
  FileBuffer buffer;
  endArrayInBuffer(buffer);
  return buffer;
}

FileBuffer initObject() {
  FileBuffer buffer;
  initObjectInBuffer(buffer);
  return buffer;
}

FileBuffer endObject() {
  FileBuffer buffer;
  endObjectInBuffer(buffer);
  return buffer;
}

void initArrayInBuffer(FileBuffer &buffer) { buffer.stream << '['; }
void endArrayInBuffer(FileBuffer &buffer) { buffer.stream << "]"; }
void initObjectInBuffer(FileBuffer &buffer) { buffer.stream << '{'; }
void endObjectInBuffer(FileBuffer &buffer) { buffer.stream << "}"; }
void endLineInBuffer(FileBuffer &buffer) { buffer.stream << ';'; }
void initConstInBuffer(FileBuffer &buffer, std::string_view constName) {
  buffer.stream << "const " << constName << "=";
}

} // namespace javascript

// ! =========== Refractoring so far =================

void saveResultsAsJson(std::string_view path, const EnergyPerFrequency &results,
                       bool referenceModel) {
  std::string outputPath = path.data();
  outputPath += "/results.js";

  File file(outputPath);

  FileBuffer fileBuffer;
  if (referenceModel) {
    file.open();
    javascript::initConstInBuffer(fileBuffer, "referenceResults");
  } else {
    file.openFileWithOverwrite();
    fileBuffer = javascript::initConst("results");
  }

  Json outputArray = Json::array();
  for (auto it = results.cbegin(); it != results.cend(); ++it) {
    Json energyData = Json::array();
    for (const float &energy : it->second) {
      energyData.push_back(energy);
    }
    Json dataPerFrequency = {{"frequency", it->first}, {"data", energyData}};
    outputArray.push_back(dataPerFrequency);
  }

  fileBuffer.acquireJsonFile(outputArray);
  javascript::endLineInBuffer(fileBuffer);
  file.write(fileBuffer);
}

void saveModelToJson(std::string_view pathToFolder, ModelInterface *model) {

  if (model == nullptr) {
    throw std::invalid_argument(
        "Model in saveModelToJson() cannot be nullptr! ");
  }

  std::string outputPath = pathToFolder.data();
  outputPath += "/model.js";

  File file(outputPath);

  Json outputJson = Json::array();
  for (const objects::TriangleObj &triangle : model->triangles()) {
    Json currentTriangle = {{"point1",
                             {{"x", triangle.point1().x()},
                              {"y", triangle.point1().y()},
                              {"z", triangle.point1().z()}}},
                            {"point2",
                             {{"x", triangle.point2().x()},
                              {"y", triangle.point2().y()},
                              {"z", triangle.point2().z()}}},
                            {"point3",
                             {{"x", triangle.point3().x()},
                              {"y", triangle.point3().y()},
                              {"z", triangle.point3().z()}}}};
    outputJson.push_back(currentTriangle);
  }

  FileBuffer buffer = javascript::initConst("model");
  // TODO: Unify this
  file.write(buffer);
  buffer.acquireJsonFile(outputJson);
  javascript::endLineInBuffer(buffer);
  file.write(buffer);
}

void PositionTrackerInterface::printItself(std::ostream &os) const noexcept {
  os << "Position Tracker Class Inteface";
}

JsonPositionTracker::JsonPositionTracker(std::string_view path)
    : path_(path), file_(path) {

  path_ += "/trackingData.js";
  outFile_.open(path_);

  if (!outFile_.good()) {
    std::stringstream ss;
    ss << "Invalid path given to: \n"
       << *this << "make sure that file at path: \"" << path_ << "\" exist";
    throw std::invalid_argument(ss.str());
  }

  outFile_ << "const trackingData = [";
  outFile_.close();
};

void JsonPositionTracker::initializeNewFrequency(float frequency) {
  std::ofstream outFile;
  outFile.open(path_, std::ios_base::app);
  if (!outFile.good()) {
    std::stringstream errorStream;
    errorStream << "Invalid Path given to: " << *this
                << "at initializenewFrequency()\n"
                << "path: " << path_;
    throw std::invalid_argument(errorStream.str());
  }

  outFile << "{\"frequency\":" << frequency << ",\n"
          << "\"trackings\":[";
  outFile.close();
}

void JsonPositionTracker::printItself(std::ostream &os) const noexcept {
  os << "Json Position Tracker\n"
     << "Path: " << path_ << "\n"
     << "current tracking: \n";
  int currentHitData = 0;
  for (const core::RayHitData &hitData : currentTracking_) {
    os << "HitData number " << currentHitData << "\n" << hitData << "\n";
  }
}

void JsonPositionTracker::initializeNewTracking() { currentTracking_.clear(); }
void JsonPositionTracker::addNewPositionToCurrentTracking(
    const core::RayHitData &hitData) {
  currentTracking_.push_back(hitData);
}
void JsonPositionTracker::endCurrentFrequency() {
  std::ofstream outFile;
  outFile.open(path_, std::ios_base::app);
  if (!outFile.good()) {
    std::stringstream errorStream;
    errorStream << "Invalid Path given to: " << *this
                << "at initializenewFrequency()\n"
                << "path: " << path_;
    throw std::invalid_argument(errorStream.str());
  }

  outFile << "],},";
  outFile.close();
}
void JsonPositionTracker::endCurrentTracking() {
  if (currentTracking_.size() > 1) {
    using Json = nlohmann::json;
    Json trackingJson = Json::array();
    for (const core::RayHitData &hitData : currentTracking_) {
      Json hitDataJson = {
          {"origin",
           {{"x", hitData.origin().x()},
            {"y", hitData.origin().y()},
            {"z", hitData.origin().z()}}},
          {"direction",
           {
               {"x", hitData.direction().x()},
               {"y", hitData.direction().y()},
               {"z", hitData.direction().z()},
           }},
          {"energy", hitData.energy()},
          {"length",
           (hitData.origin() - hitData.collisionPoint()).magnitude()}};
      trackingJson.push_back(hitDataJson);
    }

    outFile_.open(path_, std::ios_base::app);
    outFile_ << trackingJson << ",\n";
    outFile_.close();
  };
}

void JsonPositionTracker::save() {
  outFile_.open(path_, std::ios_base::app);
  outFile_ << "]";
  outFile_.close();
}

void JsonPositionTracker::switchToReferenceModel() {
  std::ofstream outFile;
  outFile.open(path_, std::ios_base::app);
  outFile << "\n"
          << "const referenceTrackingData = [";
}

JsonSampledPositionTracker::JsonSampledPositionTracker(
    std::string_view path, int numOfRaysSquared, int numOfVisibleRaysSquared)
    : tracker_(path.data()), numOfRaysSquared_(numOfRaysSquared),
      numOfVisibleRaysSquared_(numOfVisibleRaysSquared) {

  std::stringstream errorStream;
  if (numOfRaysSquared_ < 1) {
    errorStream << "Number of Rays squared cannot be less than 1\n";
  }
  if (numOfVisibleRaysSquared_ < 1) {
    errorStream << "Number of Visible Rays squared cannot be less than 1";
  }

  std::string errorMsg = errorStream.str();
  if (!errorMsg.empty()) {
    std::stringstream outputErrorStream;
    outputErrorStream << "Error occurred in: " << *this << "\n" << errorMsg;
    throw std::invalid_argument(outputErrorStream.str());
  }
};

void JsonSampledPositionTracker::initializeNewFrequency(float frequency) {
  tracker_.initializeNewFrequency(frequency);
}

void JsonSampledPositionTracker::initializeNewTracking() {
  ++currentNumberOfTracking_;
  if (isSampling()) {
    tracker_.initializeNewTracking();
  }
}

void JsonSampledPositionTracker::addNewPositionToCurrentTracking(
    const core::RayHitData &hitData) {
  if (isSampling()) {
    tracker_.addNewPositionToCurrentTracking(hitData);
  }
}

void JsonSampledPositionTracker::endCurrentFrequency() {
  tracker_.endCurrentFrequency();
}

void JsonSampledPositionTracker::endCurrentTracking() {
  if (isSampling()) {
    tracker_.endCurrentTracking();
  }
}

void JsonSampledPositionTracker::save() { tracker_.save(); }

void JsonSampledPositionTracker::switchToReferenceModel() {
  tracker_.switchToReferenceModel();
}

void JsonSampledPositionTracker::printItself(std::ostream &os) const noexcept {
  os << "Json Sampled position tracking\n"
     << "current numebr of trackings: " << currentNumberOfTracking_ << " / "
     << (numOfRaysSquared_ * numOfRaysSquared_) << "\n"
     << "number of visible rays: "
     << (numOfVisibleRaysSquared_ * numOfVisibleRaysSquared_) << "\n"
     << "Json postion tracker: " << tracker_;
}

bool JsonSampledPositionTracker::isSampling() const {
  int xIndex = currentNumberOfTracking_ % numOfRaysSquared_;
  int yIndex = currentNumberOfTracking_ / numOfRaysSquared_;
  int moduloDivider = numOfRaysSquared_ / numOfVisibleRaysSquared_;
  if (xIndex % moduloDivider == 0 && yIndex % moduloDivider == 0) {
    return true;
  }
  return false;
}
// exports |energyCollectors| as string representation to |path|
void CollectorsTrackerToJson::save(const Collectors &energyCollectors,
                                   std::string_view path) {

  std::string resultPath = path.data();
  resultPath += "/energyCollectors.js";
  std::ofstream outFile(resultPath);
  if (!outFile.good()) {
    std::stringstream errorStream;
    errorStream << "File at given path to: " << *this
                << "check if file at: " << resultPath << "exist!";
    throw std::invalid_argument(errorStream.str());
  }

  using Json = nlohmann::json;
  Json outArray = Json::array();
  int currentCollectorNumber = 0;
  for (const auto &collector : energyCollectors) {
    core::Vec3 collectorOrigin = collector->getOrigin();
    float radius = collector->getRadius();
    Json energyCollector = {{"number", currentCollectorNumber},
                            {"x", collectorOrigin.x()},
                            {"y", collectorOrigin.y()},
                            {"z", collectorOrigin.z()},
                            {"radius", radius}};
    outArray.push_back(energyCollector);
    ++currentCollectorNumber;
  }

  outFile << "const energyCollectors = " << outArray.dump(1);
  outFile.close();
}

void CollectorsTrackerInterface::printItself(std::ostream &os) const noexcept {
  os << "Collectors Tracker Class Interface\n";
}

void CollectorsTrackerToJson::printItself(std::ostream &os) const noexcept {
  os << "Json Collectors Tracker \n";
}
} // namespace trackers