/*
 * Kotlin
 *
 * Copyright 2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
 
plugins {
    id("com.microej.gradle.mock")
}

microej {
    skippedCheckers = "changelog,readme,license,nullanalysis"
    additionalFilesDir.set(rootProject.layout.projectDirectory)
}

dependencies {

}